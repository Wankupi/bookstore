#include "BookStoreLexer.h"
#include <cctype>

static std::vector<std::string> splitCommand(std::string &&cmd) {
	for (char c : cmd) if (c != ' ' && isspace(c)) throw param_exception();
	std::vector<std::string> r;
	std::istringstream is(std::move(cmd));
	std::string t;
	while (is >> t)
		r.emplace_back(std::move(t));
	return r;
}

static void check_userid_password(std::string const &str) {
	if (str.length() > 30) throw book_exception("string is too long");
	for (auto const c : str) {
		if (!isalnum(c) && c != '_')
			throw book_exception("string contains not excepted characters");
	}
}

static void check_username(std::string const &str) {
	if (str.length() > 30) throw book_exception("string is too long");
	for (auto c : str)
		if (isspace(c))
			throw book_exception("string contain space characters");
}

static std::pair<std::string, std::string> splitKeyValue(std::string const &s) {
	if (s[0] != '-') throw book_exception("-key=value param wrong");
	int eq = s.find('=');
	if (eq == std::string::npos) throw book_exception("-key=value param wrong");
	auto key = s.substr(1, eq - 1);
	auto value = s.substr(eq + 1);
	if (key.empty() || value.empty()) throw book_exception("empty -key=value");
	return {key, value};
}

static void remove_quote(std::string &s) {
	if (s.length() < 2) throw book_exception("the string to remove quote too short");
	if (s.front() != '\"' || s.back() != '\"') throw book_exception("the string is not quoted");
	s.pop_back();
	s.erase(s.begin());
}

static void check_ISBN(std::string const &str) {
	if (str.length() > 20) throw book_exception("string is too long");
	for (auto c : str)
		if (isspace(c))
			throw book_exception("string contain space characters");
}

static void check_name_author_keyword(std::string const &str) {
	if (str.empty()) throw book_exception("empty string");
	if (str.length() > 60) throw book_exception("string is too long");
	for (auto c : str)
		if (isspace(c) || c == '\"')
			throw book_exception("string contain space characters");
}

static int check_quantity(std::string const &str) {
	if (str.length() > 10) throw book_exception("string is too long");
	if (str.length() > 1 && str.front() == '0') throw book_exception("leading zero");
	int x = 0;
	for (auto c : str) {
		if (c < '0' || c > '9') throw book_exception("not number character");
		x = x * 10 + c - '0';
	}
	return x;
}

static double check_price_cost(std::string const &str) {
	if (str.length() > 13) throw book_exception("string is too long");
	if (str.front() == '.' || str.back() == '.') throw book_exception("except number surround dot");
	if (str.front() == '0' && str.length() > 1 && str[1] != '.') throw book_exception("leading zero");
	double x = 0, xi = 1;
	bool dot = false;
	for (auto c : str) {
		if (c == '.') {
			if (dot) throw book_exception("two dots");
			dot = true;
		}
		else if ('0' <= c && c <= '9') {
			if (dot)
				x += (c - '0') * (xi /= 10);
			else
				x = x * 10 + (c - '0');
		}
		else throw book_exception("not number character");
	}
	if (x == 0) throw book_exception("price/cost equals to 0");
	return x;
}

void printBook(Book const &book) {
	std::cout << book;
}

void printBookNothing() {
	std::cout << '\n';
}

int BookStoreLexer::processLine(std::string cmd) {
	auto argv = splitCommand(std::move(cmd));
	if (argv.empty()) return 0;
	if (argv[0] == "su")
		func_su(argv);
	else if (argv[0] == "logout") func_logout(argv);
	else if (argv[0] == "register") func_register(argv);
	else if (argv[0] == "passwd") func_passwd(argv);
	else if (argv[0] == "useradd") func_useradd(argv);
	else if (argv[0] == "delete") func_delete(argv);
	else if (argv[0] == "show") {
		if (argv.size() >= 2 && argv[1] == "finance")
			func_show_finance(argv);
		else func_show(argv);
	}
	else if (argv[0] == "buy") func_buy(argv);
	else if (argv[0] == "select") func_select(argv);
	else if (argv[0] == "modify") func_modify(argv);
	else if (argv[0] == "import") func_import(argv);
	else if (argv[0] == "quit" || argv[0] == "exit") {
		if (argv.size() > 1) throw param_exception();
		return 1;
	}
	else if (argv[0] == "log") func_log(argv);
	else
		throw param_exception();
	return 0;
}

void BookStoreLexer::func_su(const std::vector<std::string> &argv) {
	if (argv.size() == 2) {
		check_userid_password(argv[1]);
		store.login(argv[1], {});
	}
	else if (argv.size() == 3) {
		check_userid_password(argv[1]);
		check_userid_password(argv[2]);
		store.login(argv[1], argv[2]);
	}
	else throw param_exception();
}

void BookStoreLexer::func_logout(const std::vector<std::string> &argv) {
	if (argv.size() > 1) throw book_exception("too many arguments");
	store.logout();
}

void BookStoreLexer::func_register(const std::vector<std::string> &argv) {
	if (argv.size() != 4) throw param_exception();
	check_userid_password(argv[1]);
	check_userid_password(argv[2]);
	check_username(argv[3]);
	store.Register(argv[1], argv[2], argv[3]);
}

void BookStoreLexer::func_passwd(const std::vector<std::string> &argv) {
	if (argv.size() == 3) {
		check_userid_password(argv[1]);
		check_userid_password(argv[2]);
		store.passwd(argv[1], {}, argv[2]);
	}
	else if (argv.size() == 4) {
		check_userid_password(argv[1]);
		check_userid_password(argv[2]);
		check_userid_password(argv[3]);
		store.passwd(argv[1], argv[2], argv[3]);
	}
	else throw param_exception();
}

void BookStoreLexer::func_useradd(const std::vector<std::string> &argv) {
	if (argv.size() != 5) throw param_exception();
	check_userid_password(argv[1]);
	check_userid_password(argv[2]);
	check_username(argv[4]);
	if (argv[3].size() != 1) throw param_exception();
	int p = argv[3][0] - '0';
	if (p != 1 && p != 3) throw param_exception();
	store.useradd(argv[1], argv[2], Privilege(p), argv[4]);
}

void BookStoreLexer::func_delete(const std::vector<std::string> &argv) {
	if (argv.size() != 2) throw param_exception();
	check_userid_password(argv[1]);
	store.userdel(argv[1]);
}

void BookStoreLexer::func_show(const std::vector<std::string> &argv) {
	if (argv.size() > 2) throw param_exception();
	auto sql = store.show();
	for (size_t i = 1; i < argv.size(); ++i) {
		auto pr = splitKeyValue(argv[i]);
		if (pr.first == "ISBN") {
			check_ISBN(pr.second);
			sql.limitISBN(pr.second);
		}
		else {
			remove_quote(pr.second);
			check_name_author_keyword(pr.second);
			if (pr.first == "name")
				sql.limitName(pr.second);
			else if (pr.first == "author")
				sql.limitAuthor(pr.second);
			else if (pr.first == "keyword") {
				if (pr.second.find('|') != std::string::npos)
					throw param_exception();
				sql.limitKey(pr.second);
			}
			else throw param_exception();
		}
	}
	sql.for_each(printBook, printBookNothing);
}

void BookStoreLexer::func_buy(const std::vector<std::string> &argv) {
	if (argv.size() != 3) throw param_exception();
	check_ISBN(argv[1]);
	int quantity = check_quantity(argv[2]);
	double cost = store.buy(argv[1], quantity);
	std::cout << cost << std::endl;
}

void BookStoreLexer::func_select(const std::vector<std::string> &argv) {
	if (argv.size() != 2) throw param_exception();
	check_ISBN(argv[1]);
	store.select(argv[1]);
}

void BookStoreLexer::func_modify(const std::vector<std::string> &argv) {
	if (argv.size() == 1) throw param_exception();
	BookModify bm;
	for (size_t i = 1; i < argv.size(); ++i) {
		auto pr = splitKeyValue(argv[i]);
		if (pr.first == "ISBN") {
			check_ISBN(pr.second);
			bm.modifyISBN(pr.second);
		}
		else if (pr.first == "price")
			bm.modifyPrice(check_price_cost(pr.second));
		else {
			remove_quote(pr.second);
			check_name_author_keyword(pr.second);
			if (pr.first == "name")
				bm.modifyName(pr.second);
			else if (pr.first == "author")
				bm.modifyAuthor(pr.second);
			else if (pr.first == "keyword")
				bm.modifyKey(pr.second);
			else throw param_exception();
		}
	}
	store.modify(bm);
}

void BookStoreLexer::func_import(const std::vector<std::string> &argv) {
	if (argv.size() != 3) throw param_exception();
	store.Import(check_quantity(argv[1]), check_price_cost(argv[2]));
}

 void BookStoreLexer::func_show_finance(const std::vector<std::string> &argv) {
	if (argv.size() > 3) throw param_exception();
	std::pair<double, double> res;
	if (argv.size() == 3) {
		int count = check_quantity(argv[2]);
		if (count == 0) std::cout << std::endl;
		else {
			res = store.show_finance(count);
			std::cout << "+ " << res.first << " - " << res.second << std::endl;
		}
	}
	else {
		res = store.show_finance();
		std::cout << "+ " << res.first << " - " << res.second << std::endl;
	}
}

 void BookStoreLexer::func_log(const std::vector<std::string> &argv) {
 }
