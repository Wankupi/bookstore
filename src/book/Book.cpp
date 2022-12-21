#include "Book.h"

std::ostream &operator<<(std::ostream &os, Book const &book) {
	// [ISBN]\t[BookName]\t[Author]\t[Keyword]\t[Price]\t[库存数量]\n
	return os << book.ISBN << '\t' << book.name << '\t' << book.author << '\t' << book.keyword << '\t' << book.price << '\t' << book.quantity << '\n';
}

#pragma pack(push, 1)
struct CntAndPrice {
	int count;
	double price;
};
#pragma pack(pop)

double BookData::buy(int id, int count) {
	CntAndPrice data{};
	std::lock_guard lock(file_lock);
	file.seekg((id - 1) * sizeof(Book) + offsetof(Book, quantity));
	file.read(reinterpret_cast<char *>(&data), sizeof(data));
	data.count -= count;
	if (data.count < 0) return -1;
	file.seekg((id - 1) * sizeof(Book) + offsetof(Book, quantity));
	file.write(reinterpret_cast<char *>(&data), sizeof(int));
	return count * data.price;
}

int BookData::Import(int id, int count) {
	int quantity = 0;
	std::lock_guard lock(file_lock);
	file.seekg((id - 1) * sizeof(Book) + offsetof(Book, quantity));
	file.read(reinterpret_cast<char *>(&quantity), sizeof(int));
	quantity += count;
	file.seekg((id - 1) * sizeof(Book) + offsetof(Book, quantity));
	file.write(reinterpret_cast<char *>(&quantity), sizeof(int));
	return quantity;
}

BookSearchResult &BookSearchResult::limitISBN(const String<20> &ISBN) {
	if (unset) {
		unset = false;
		res = books->findISBN(ISBN).res;
		return *this;
	}
	auto cur = res.begin();
	while (cur != res.end()) {
		auto p = cur++;
		if (p->ISBN != ISBN) res.erase(p);
	}
	return *this;
}

BookSearchResult &BookSearchResult::limitName(const String<60> &name) {
	if (unset) {
		unset = false;
		res = books->findName(name).res;
		return *this;
	}
	auto cur = res.begin();
	while (cur != res.end()) {
		auto p = cur++;
		if (p->name != name) res.erase(p);
	}
	return *this;
}

BookSearchResult &BookSearchResult::limitAuthor(const String<60> &aur) {
	if (unset) {
		unset = false;
		res = books->findAuthor(aur).res;
		return *this;
	}
	auto cur = res.begin();
	while (cur != res.end()) {
		auto p = cur++;
		if (p->author != aur) res.erase(p);
	}
	return *this;
}

std::set<std::string> splitKeywords(String<60> const &key) {
	std::set<std::string> res;
	for (int l = 0, r; l < key.size() && key[l]; l = r + 1) {
		r = l;
		while (r < key.size() && key[r] && key[r] != '|')
			++r;
		if (r - l == 0) throw book_exception("keywords - empty");
		auto ins_res = res.emplace(key.data() + l, r - l);
		if (!ins_res.second) throw book_exception("keywords - repeat");
	}
	return res;
}

BookSearchResult &BookSearchResult::limitKey(const String<60> &key) {
	if (unset) {
		unset = false;
		res = books->findKey(key).res;
		return *this;
	}
	auto cur = res.begin();
	while (cur != res.end()) {
		auto p = cur++;
		auto words = splitKeywords(p->keyword);
		if (words.insert(std::string(key)).second)
			res.erase(p);
	}
	return *this;
}

void BookSearchResult::for_each(void (*func)(Book const &), void (*emptyFun)()) {
	if (unset)
		res = books->findAll().res;
	for (auto const &book : res)
		func(book);
	if (res.empty())
		emptyFun();
}

BookModify::BookModify(BookModify &&rhs) noexcept
	: BookModify() {
	std::swap(Isbn, rhs.Isbn);
	std::swap(Name, rhs.Name);
	std::swap(Author, rhs.Author);
	std::swap(Key, rhs.Key);
	std::swap(Price, rhs.Price);
}

BookModify::~BookModify() {
	delete Isbn;
	delete Name;
	delete Author;
	delete Key;
	delete Price;
}

BookModify &BookModify::modifyISBN(const String<20> &ISBN) {
	if (Isbn) throw book_exception("modify - repeat");
	Isbn = new String<20>(ISBN);
	return *this;
}

BookModify &BookModify::modifyName(const String<60> &name) {
	if (Name) throw book_exception("modify - repeat");
	Name = new String<60>(name);
	return *this;
}

BookModify &BookModify::modifyAuthor(const String<60> &author) {
	if (Author) throw book_exception("modify - repeat");
	Author = new String<60>(author);
	return *this;
}

BookModify &BookModify::modifyKey(const String<60> &key) {
	if (Key) throw book_exception("modify - repeat");
	Key = new String<60>(key);
	return *this;
}

BookModify &BookModify::modifyPrice(double price) {
	if (Price) throw book_exception("modify - repeat");
	Price = new double(price);
	return *this;
}


BookSearchResult Books::findISBN(const String<20> &ISBN) {
	auto v = ISBNs.find(ISBN);
	BookSearchResult ret;
	for (auto id : v)
		ret.res.emplace(db.read(id));
	return ret;
}

BookSearchResult Books::findName(const String<60> &name) {
	auto v = Names.find(name);
	BookSearchResult ret;
	for (auto id : v)
		ret.res.emplace(db.read(id));
	return ret;
}

BookSearchResult Books::findAuthor(const String<60> &author) {
	auto v = Authors.find(author);
	BookSearchResult ret;
	for (auto id : v)
		ret.res.emplace(db.read(id));
	return ret;
}

BookSearchResult Books::findKey(const String<60> &key) {
	auto v = Keys.find(key);
	BookSearchResult ret;
	for (auto id : v)
		ret.res.emplace(db.read(id));
	return ret;
}

BookSearchResult Books::Query() {
	return BookSearchResult{this};
}

BookSearchResult Books::findAll() {
	int n = db.size();
	BookSearchResult ret;
	for (int i = 1; i <= n; ++i)
		ret.res.emplace(db.read(i));
	return ret;
}

void Books::for_each(void (*func)(Book const &), void (*emptyFun)()) {
	int n = db.size();
	for (int i = 1; i <= n; ++i)
		func(db.read(i));
	if (n == 0) emptyFun();
}

double Books::buy(const String<20> &ISBN, int quantity) {
	if (quantity <= 0) return -1;
	auto v = ISBNs.find(ISBN);
	if (v.empty()) return -1;
	return db.buy(v[0], quantity);
}

int Books::getId(const String<20> &ISBN) {
	std::lock_guard lock(cnt_lock);
	auto v = ISBNs.find(ISBN);
	if (!v.empty()) return v.front();
	int id = db.insert(Book(ISBN));
	ISBNs.insert(ISBN, id);
	// do not set index of other data
	return id;
}

bool Books::modifyApply(int id, const BookModify &modify) {
	if (modify.Isbn && !ISBNs.find(*modify.Isbn).empty())
		return false;
	// because split may throw, so do it before
	std::set<std::string> keys2;
	if (modify.Key) keys2 = splitKeywords(*modify.Key);
	std::lock_guard lock(book_lock[id]);
	Book book = db.read(id);
	if (modify.Isbn) {
		std::lock_guard lock_isbn(cnt_lock);
		ISBNs.erase(book.ISBN, id);
		ISBNs.insert(book.ISBN = *modify.Isbn, id);
	}
	if (modify.Name) {
		if (!book.name.allzero()) Names.erase(book.name, id);
		Names.insert(book.name = *modify.Name, id);
	}
	if (modify.Author) {
		if (!book.author.allzero()) Authors.erase(book.author, id);
		Authors.insert(book.author = *modify.Author, id);
	}
	if (modify.Key) {
		auto keys1 = splitKeywords(book.keyword);
		for (auto &s : keys1) {
			if (keys2.find(s) == keys2.end())
				Keys.erase(s, id);
		}
		for (auto &s : keys2) {
			if (keys1.find(s) == keys1.end())
				Keys.insert(s, id);
		}
		book.keyword = *modify.Key;
	}
	if (modify.Price) book.price = *modify.Price;
	db.write(id, book);
	return true;
}

int Books::Import(int id, int quantity, double total_cost) {
	if (quantity <= 0 || total_cost <= 0) return 0;
	return db.Import(id, quantity);
}
