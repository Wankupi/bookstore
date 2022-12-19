#pragma once
#ifndef BOOKSTORE_STOREBRANCH_H
#define BOOKSTORE_STOREBRANCH_H
#include "../book/Book.h"
#include "../finance/Finance.h"
#include "../user/User.h"
#include <unordered_map>
#include <vector>

class StoreBranch {
public:
	friend int main();
	StoreBranch(Users &users, Books &books, Finance &finance) : users(users), books(books), finance(finance) {}
	~StoreBranch();
	void login(String<30> const &UserID, String<30> const &password);
	void logout();
	void Register(String<30> const &UserID, String<30> const &password, String<30> const &Username);
	void passwd(String<30> const &UserId, String<30> const &cur_pwd, String<30> const &new_pwd);
	void useradd(String<30> const &UserID, String<30> const &password, Privilege privilege, String<30> Username);
	void userdel(String<30> const &UserID);

	BookSearchResult show();
	double buy(String<20> const &ISBN, int quantity);
	void select(String<20> const &ISBN);
	void modify(BookModify const &modify);
	void Import(int quantity, double total_cost);

	std::pair<double, double> show_finance();
	std::pair<double, double> show_finance(int count);
private:
	Privilege currentPrivilege() { return st.empty() ? Privilege::logout : st.back().first.privilege; }

private:
	Users &users;
	Books &books;
	Finance &finance;
	std::vector<std::pair<UserInfo, int>> st;
};

#endif // BOOKSTORE_STOREBRANCH_H
