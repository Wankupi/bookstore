#pragma once
#ifndef BOOKSTORE_STOREBRANCH_H
#define BOOKSTORE_STOREBRANCH_H
#include "user/User.h"
#include <stack>
#include <unordered_map>

class StoreBranch {
public:
	friend int main();
	StoreBranch(Users &users) : cmd(users) {}
	void login(String<30> const &UserID, String<30> const &password);
	void logout();
	void Register(String<30> const &UserID, String<30> const &password, String<30> const &Username);
	void passwd(String<30> const &UserId, String<30> const &cur_pwd, String<30> const &new_pwd);
	void useradd(String<30> const &UserID, String<30> const &password, Privilege privilege, String<30> Username);
	void userdel(String<30> const &UserID);

private:
	Privilege currentPrivilege() { return st.empty() ? Privilege::logout : st.top().first.privilege; }
private:
	Users &cmd;
	std::stack<std::pair<UserInfo, int>> st;
};

#endif // BOOKSTORE_STOREBRANCH_H
