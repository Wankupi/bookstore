#include "StoreBranch.h"

void StoreBranch::login(const String<30> &UserID, const String<30> &password) {
	auto r = users.login(UserID, password, currentPrivilege());
	if (r.id == 0) throw book_exception();
	st.emplace_back(r, 0);
}

void StoreBranch::logout() {
	if (st.empty()) throw book_exception();
	users.logout(st.back().first.id);
	st.pop_back();
}

void StoreBranch::Register(const String<30> &UserID, const String<30> &password, const String<30> &Username) {
	if (!users.Register(UserID, password, Username))
		throw book_exception();
}

void StoreBranch::passwd(const String<30> &UserId, const String<30> &cur_pwd, const String<30> &new_pwd) {
	if (!users.passwd(UserId, cur_pwd, new_pwd, currentPrivilege()))
		throw book_exception();
}

void StoreBranch::useradd(const String<30> &UserID, const String<30> &password, Privilege privilege, String<30> Username) {
	if (!users.useradd(UserID, password, privilege, Username, currentPrivilege()))
		throw book_exception();
}

void StoreBranch::userdel(const String<30> &UserID) {
	if (!users.userdel(UserID, currentPrivilege()))
		throw book_exception();
}

BookSearchResult StoreBranch::show() {
	if (currentPrivilege() < Privilege::customer)
		throw book_exception();
	return books.Query();
}

void StoreBranch::buy(String<20> const &ISBN, int quantity) {
	if (currentPrivilege() < Privilege::customer)
		throw book_exception();
	if (quantity <= 0) throw book_exception();
	double cost = books.buy(ISBN, quantity);
	if (cost < 0) throw book_exception();
}

void StoreBranch::select(String<20> const &ISBN) {
	if (currentPrivilege() < Privilege::staff)
		throw book_exception();
	int id = books.getId(ISBN);
	if (id == 0) throw book_exception(); // will not run
	st.back().second = id;
}

void StoreBranch::modify(BookModify const &modify) {
	if (currentPrivilege() < Privilege::staff)
		throw book_exception();
	if (st.back().second == 0) throw book_exception();
	bool stat = books.modifyApply(st.back().second, modify);
	if (!stat) throw book_exception();
}

void StoreBranch::Import(int quantity, double total_cost) {
	if (currentPrivilege() < Privilege::staff)
		throw book_exception();
	if (st.back().second == 0) throw book_exception();
	int count_after_import = books.Import(st.back().second, quantity, total_cost);
	if (count_after_import == 0)
		throw book_exception();
}
