#include "Book.h"

std::ostream &operator<<(std::ostream &os, Book const &book) {
	// [ISBN]\t[BookName]\t[Author]\t[Keyword]\t[Price]\t[库存数量]\n
	return os << book.ISBN << '\t' << book.name << '\t' << book.author << '\t' << book.keyword << '\t' << book.price << '\t' << book.quantity << '\n';
}

double BookData::buy(int id, int count) {
	std::pair<int, double> data;
	std::lock_guard lock(file_lock);
	file.seekg((id - 1) * sizeof(Book) + offsetof(Book, quantity));
	file.read(reinterpret_cast<char *>(&data), sizeof(data));
	data.first -= count;
	if (data.first < 0) return -1;
	file.seekg((id - 1) * sizeof(Book) + offsetof(Book, quantity));
	file.write(reinterpret_cast<char *>(&data), sizeof(int));
	return count * data.second;
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

static std::set<std::string> splitKeywords(String<60> const &key) {
	std::set<std::string> res;
	for (int l = 0, r = 0; l < key.size() && key[l]; l = r + 1) {
		r = l;
		while (r < key.size() && key[r] != '|')
			++r;
		if (r - l == 0) throw book_exception();
		auto ins_res = res.emplace(key.data() + l, r - l);
		if (ins_res.second) throw book_exception();
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
	if (Isbn) throw book_exception();
	Isbn = new String<20>(ISBN);
	return *this;
}

BookModify &BookModify::modifyName(const String<60> &name) {
	if (Name) throw book_exception();
	Name = new String<60>(name);
	return *this;
}

BookModify &BookModify::modifyAuthor(const String<60> &author) {
	if (Author) throw book_exception();
	Author = new String<60>(author);
	return *this;
}

BookModify &BookModify::modifyKey(const String<60> &key) {
	if (Key) throw book_exception();
	Key = new String<60>(key);
	return *this;
}

BookModify &BookModify::modifyPrice(double price) {
	if (Price) throw book_exception();
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

double Books::buy(const String<20> &ISBN, int quantity) {
	if (quantity <= 0) return -1;
	int id = getId(ISBN);
	if (!id) return 0;
	return db.buy(id, quantity);
}

int Books::getId(const String<20> &ISBN) {
	auto v = ISBNs.find(ISBN);
	return v.empty() ? 0 : v.front();
}

bool Books::modifyApply(int id, const BookModify &modify) {
	if (modify.Isbn && !ISBNs.find(*modify.Isbn).empty())
		return false;
	std::lock_guard lock(book_lock[id]);
	Book book = db.read(id);
	if (modify.Isbn) {
		ISBNs.erase(book.ISBN, id);
		ISBNs.erase(book.ISBN = *modify.Isbn, id);
	}
	if (modify.Name) {
		Names.erase(book.name, id);
		Names.insert(book.name = *modify.Name, id);
	}
	if (modify.Author) {
		Authors.erase(book.author, id);
		Authors.insert(book.author = *modify.Author, id);
	}
	if (modify.Key) {
		auto keys1 = splitKeywords(book.keyword),
			 keys2 = splitKeywords(*modify.Key);
		for (auto &s : keys1) {
			if (keys2.find(s) != keys2.end())
				Keys.erase(s, id);
		}
		for (auto &s : keys2) {
			if (keys1.find(s) != keys1.end())
				Keys.insert(s, id);
		}
		book.keyword = *modify.Key;
	}
	if (modify.Price) book.price = *modify.Price;
	db.write(id, book);
}

int Books::Import(int id, int quantity, double total_cost) {
	if (quantity <= 0 || total_cost <= 0) return 0;
	return db.Import(id, quantity);
}
