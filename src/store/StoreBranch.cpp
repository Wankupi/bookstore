#include "StoreBranch.h"

void StoreBranch::login(const String<30> &UserID, const String<30> &password) {
	auto r = users.login(UserID, password, currentPrivilege());
	if (r.id == 0) throw book_exception("login - user no found");
	st.emplace_back(r, 0);
}

void StoreBranch::logout() {
	if (st.empty()) throw book_exception("logout - no user logged in");
	users.logout(st.back().first.id);
	st.pop_back();
}

void StoreBranch::Register(const String<30> &UserID, const String<30> &password, const String<30> &Username) {
	if (!users.Register(UserID, password, Username))
		throw book_exception("register - failed");
}

void StoreBranch::passwd(const String<30> &UserId, const String<30> &cur_pwd, const String<30> &new_pwd) {
	if (!users.passwd(UserId, cur_pwd, new_pwd, currentPrivilege()))
		throw book_exception("passwd - failed");
}

void StoreBranch::useradd(const String<30> &UserID, const String<30> &password, Privilege privilege, String<30> Username) {
	if (!users.useradd(UserID, password, privilege, Username, currentPrivilege()))
		throw book_exception("useradd - failed");
}

void StoreBranch::userdel(const String<30> &UserID) {
	if (!users.userdel(UserID, currentPrivilege()))
		throw book_exception("userdel - failed");
}

BookSearchResult StoreBranch::show() {
	if (currentPrivilege() < Privilege::customer)
		throw book_exception("show - not enough privilege");
	return books.Query();
}

double StoreBranch::buy(String<20> const &ISBN, int quantity) {
	if (currentPrivilege() < Privilege::customer)
		throw book_exception("buy - not enough privilege");
	if (quantity < 0) throw book_exception("buy - quantity <= 0");
	double cost = books.buy(ISBN, quantity);
	if (cost < 0) throw book_exception("buy - failed");
	finance.log_buy(cost);
	return cost;
}

void StoreBranch::select(String<20> const &ISBN) {
	if (currentPrivilege() < Privilege::staff)
		throw book_exception("select - not enough privilege");
	int id = books.getId(ISBN);
	if (id == 0) throw book_exception("select - book not found"); // will not run
	st.back().second = id;
}

void StoreBranch::modify(BookModify const &modify) {
	if (currentPrivilege() < Privilege::staff)
		throw book_exception("modify - not enough privilege");
	if (st.back().second == 0) throw book_exception("modify - have not selected");
	bool stat = books.modifyApply(st.back().second, modify);
	if (!stat) throw book_exception("modify - failed");
}

void StoreBranch::Import(int quantity, double total_cost) {
	if (currentPrivilege() < Privilege::staff)
		throw book_exception("import - not enough privilege");
	if (st.back().second == 0) throw book_exception("import - have not selected");
	int count_after_import = books.Import(st.back().second, quantity, total_cost);
	if (count_after_import == 0)
		throw book_exception("import - failed");
	finance.log_import(total_cost);
}

std::pair<double, double> StoreBranch::show_finance() {
	if (currentPrivilege() < Privilege::admin)
		throw book_exception("show finance - not enough privilege");
	return finance.show();
}

std::pair<double, double> StoreBranch::show_finance(int count) {
	if (currentPrivilege() < Privilege::admin)
		throw book_exception("show finance - not enough privilege");
	auto r = finance.show(count);
	if (r.first < 0) throw book_exception("show finance - failed");
	return r;
}

StoreBranch::~StoreBranch() {
	while (!st.empty()) {
		users.logout(st.back().first.id);
		st.pop_back();
	}
}
