#pragma once
#ifndef BOOKSTORE_BOOK_H
#define BOOKSTORE_BOOK_H
#include "../base.h"
#include "../file/FileMap.h"
#include <set>

;
#pragma pack(push, 1)
struct Book {
	Book() = default;
	explicit Book(String<20> const &ISBN) : ISBN(ISBN) {}
	Book(Book const &) = default;
	Book(Book &&) = default;
	String<20> ISBN{};
	String<60> name{};
	String<60> author{};
	String<60> keyword{};
	int quantity{};
	double price{};
	bool operator<(Book const &B) const { return ISBN < B.ISBN; }
};
#pragma pack(pop)

std::ostream &operator<<(std::ostream &os, Book const &book);

class BookData : public DataBase<Book, false> {
public:
	using DataBase<Book, false>::DataBase;
	/**
	 * @param id
	 * @param count the number of books to buy. do no check `count > 0`.
	 * @return the cost to buy or -1 if failed.
	 */
	double buy(int id, int count);
	/**
	 * @param id
	 * @param count the number of books to import. do no check `count > 0`.
	 * @return the quantity of books after import.
	 */
	int Import(int id, int count);
};

class Books;
class BookSearchResult {
	friend class Books;

public:
	BookSearchResult &limitISBN(String<20> const &ISBN);
	BookSearchResult &limitName(String<60> const &name);
	BookSearchResult &limitAuthor(String<60> const &author);
	BookSearchResult &limitKey(String<60> const &key);
	auto begin() { return res.begin(); }
	auto end() { return res.end(); }
	void for_each(void (*)(Book const &), void (*)());

private:
	explicit BookSearchResult(Books *books) : books(books), unset(true) {}
	BookSearchResult() : books(nullptr), unset(false) {}

private:
	Books *books;
	bool unset;
	std::set<Book> res;
};

class BookModify {
	friend class Books;

public:
	BookModify() = default;
	BookModify(BookModify const &) = delete;
	BookModify(BookModify &&) noexcept;
	~BookModify();
	BookModify &modifyISBN(String<20> const &ISBN);
	BookModify &modifyName(String<60> const &name);
	BookModify &modifyAuthor(String<60> const &author);
	BookModify &modifyKey(String<60> const &key);
	BookModify &modifyPrice(double price);

private:
	String<20> *Isbn = nullptr;
	String<60> *Name = nullptr;
	String<60> *Author = nullptr;
	String<60> *Key = nullptr;
	double *Price = nullptr;
};

class Books {
public:
	Books(std::string const &file_book, std::string const &file_ISBN, std::string const &file_Name, std::string const &file_Author, std::string const &file_key)
		: db(file_book), ISBNs(file_ISBN), Names(file_Name), Authors(file_Author), Keys(file_key) {}
	BookSearchResult findISBN(String<20> const &ISBN);
	BookSearchResult findName(String<60> const &name);
	BookSearchResult findAuthor(String<60> const &author);
	BookSearchResult findKey(String<60> const &key);
	/**
	 * @return a operator object to do queries.
	 */
	BookSearchResult Query();
	BookSearchResult findAll();
	void for_each(void (*)(Book const &), void (*)());
	/**
	 * @return the cost to buy these books. -1 if failed.
	 */
	double buy(String<20> const &ISBN, int quantity);
	/**
	 * @return id of the book. If not found then create a new book with ISBN.
	 */
	int getId(String<20> const &ISBN);
	/**
	 * @return whether modify sucessfully.
	 * @attention it will fail when new_isbn already exists ( even if same with itself )
	 * or keywords has repeat words.
	 */
	bool modifyApply(int id, BookModify const &modify);
	/**
	 * @return the quantity of books after import. 0 if failed.
	 * @attention it will fail when number is not a positive integer
	 * or total_cost is not positive real number.
	 */
	int Import(int id, int number, double total_cost);

private:
	BookData db;
	FileMap<String<20>, int> ISBNs;
	FileMap<String<60>, int> Names;
	FileMap<String<60>, int> Authors;
	FileMap<String<60>, int> Keys;
	std::unordered_map<int, std::mutex> book_lock;
	std::mutex cnt_lock;
};

#endif // BOOKSTORE_BOOK_H
