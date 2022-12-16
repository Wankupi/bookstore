#include "StoreBranch.h"

void StoreBranch::login(const String<30> &UserID, const String<30> &password) {
	auto r = cmd.login(UserID, password, currentPrivilege());
	if (r.id == 0) throw book_exception();
	st.emplace(r, 0);
}

void StoreBranch::logout() {
	if (st.empty()) throw book_exception();
	st.pop();
}

void StoreBranch::Register(const String<30> &UserID, const String<30> &password, const String<30> &Username) {
	if (!cmd.Register(UserID, password, Username))
		throw book_exception();
}

void StoreBranch::passwd(const String<30> &UserId, const String<30> &cur_pwd, const String<30> &new_pwd) {
	if (!cmd.passwd(UserId, cur_pwd, new_pwd, currentPrivilege()))
		throw book_exception();
}

void StoreBranch::useradd(const String<30> &UserID, const String<30> &password, Privilege privilege, String<30> Username) {
	if (!cmd.useradd(UserID, password, privilege, Username, currentPrivilege()))
		throw book_exception();
}

void StoreBranch::userdel(const String<30> &UserID) {
	if (!cmd.userdel(UserID, currentPrivilege()))
		throw book_exception();
}
