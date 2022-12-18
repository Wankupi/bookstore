#pragma once
#ifndef BOOKSTORE_STOREBRANCH_H
#define BOOKSTORE_STOREBRANCH_H
#include "user/User.h"
#include "book/Book.h"
#include <vector>
#include <unordered_map>

class StoreBranch {
public:
	friend int main();
	StoreBranch(Users &users, Books &books) : users(users), books(books) {}
	void login(String<30> const &UserID, String<30> const &password);
	void logout();
	void Register(String<30> const &UserID, String<30> const &password, String<30> const &Username);
	void passwd(String<30> const &UserId, String<30> const &cur_pwd, String<30> const &new_pwd);
	void useradd(String<30> const &UserID, String<30> const &password, Privilege privilege, String<30> Username);
	void userdel(String<30> const &UserID);

	BookSearchResult show();
	void buy(String<20> const &ISBN, int quantity);
	void select(String<20> const &ISBN);
	void modify(BookModify const &modify);
	void Import(int quantity, double total_cost);

private:
	Privilege currentPrivilege() { return st.empty() ? Privilege::logout : st.back().first.privilege; }
private:
	Users &users;
	Books &books;
	std::vector<std::pair<UserInfo, int>> st;
};

#endif // BOOKSTORE_STOREBRANCH_H
