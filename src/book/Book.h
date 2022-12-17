#pragma once
#ifndef BOOKSTORE_BOOK_H
#define BOOKSTORE_BOOK_H
#include "../base.h"
#include "../file/FileMap.h"
#include <set>

#pragma pack(push, 1)
struct Book {
	Book() = default;
	explicit Book(std::string const &_ISBN) : ISBN(_ISBN) {}
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
	double buy(int id, int count);
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
	BookSearchResult Query();

	double buy(String<20> const &ISBN, int quantity);
	int getId(String<20> const &ISBN);
	bool modifyApply(int id, BookModify const &modify);
	int Import(int id, int number, double total_cost);

private:
	BookData db;
	FileMap<String<20>, int> ISBNs;
	FileMap<String<60>, int> Names;
	FileMap<String<60>, int> Authors;
	FileMap<String<60>, int> Keys;
	std::unordered_map<int, std::mutex> book_lock;
};

#endif // BOOKSTORE_BOOK_H
