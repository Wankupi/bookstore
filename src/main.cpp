#include "InstanceDaemon.h"
#include "StoreBranch.h"
#include <iostream>
InstanceDaemon instanceDaemon("running.status");

std::vector<std::string> splitCommand(std::string &&cmd) {
	std::vector<std::string> r;
	std::istringstream is(std::move(cmd));
	std::string t;
	while (is >> t)
		r.emplace_back(std::move(t));
	return r;
}

std::pair<std::string, std::string> splitKeyValue(std::string const &s) {
	if (s[0] != '-') throw book_exception("-key=value param wrong");
	int eq = s.find('=');
	if (eq == std::string::npos) throw book_exception("-key=value param wrong");
	auto key = s.substr(1, eq - 1);
	auto value = s.substr(eq + 1);
	if (value.size() >= 2 && value.front() == '\"' && value.back() == '\"') {
		value.pop_back();
		value.erase(value.begin());
	}
	if (value.size() == 0) throw book_exception("-key=value param wrong");
	return {key, value};
}

void printBook(Book const &book) {
	std::cout << book;
}
void printBookNothing() {
	std::cout << '\n';
}

int main() {
	if (!instanceDaemon.status()) {
		std::cout << "Failed to start, another program instance may be running." << std::endl
				  << "Or you could delete the file 'running.status'." << std::endl;
	}
	else {
		try {
			std::cout << std::fixed << std::setprecision(2);
			Users users("users/users.db", "users/users.idx");
			Books books("book/books.db", "book/ISBN.idx", "book/Name.idx", "book/author.idx", "book/key.idx");
			Finance finance("pay.log");
			StoreBranch sb(users, books, finance);
			std::string cmd;
			while (std::getline(std::cin, cmd)) {
				auto argv = splitCommand(std::move(cmd));
				if (argv.empty()) continue;
				try {
					if (argv[0] == "su") {
						if (argv.size() == 2) sb.login(argv[1], {});
						else if (argv.size() == 3) sb.login(argv[1], argv[2]);
						else throw param_exception();
					}
					else if (argv[0] == "logout") {
						if (argv.size() != 1) throw param_exception{};
						sb.logout();
					}
					else if (argv[0] == "register") {
						if (argv.size() != 4) throw param_exception();
						sb.Register(argv[1], argv[2], argv[3]);
					}
					else if (argv[0] == "passwd") {
						if (argv.size() == 3) sb.passwd(argv[1], {}, argv[2]);
						else if (argv.size() == 4) sb.passwd(argv[1], argv[2], argv[3]);
						else throw param_exception();
					}
					else if (argv[0] == "useradd") {
						if (argv.size() != 5) throw param_exception();
						if (argv[3].size() != 1) throw param_exception();
						int p = argv[3][0] - '0';
						if (p != 1 && p != 3) throw param_exception();
						sb.useradd(argv[1], argv[2], Privilege(p), argv[4]);
					}
					else if (argv[0] == "delete") {
						if (argv.size() != 2) throw param_exception();
						sb.userdel(argv[1]);
					}
					else if (argv.size() >= 2 && argv[0] == "show" && (argv[1] == "Finance" || argv[1] == "finance")) {
						if (argv.size() > 3) throw param_exception();
						std::pair<double, double> res;
						if (argv.size() == 3) {
							if (argv[2].find_first_not_of("0123456789") != std::string::npos)
								throw param_exception();
							int count = std::stoi(argv[2]);
							if (count == 0) std::cout << std::endl;
							else {
								res = sb.show_finance(std::stoi(argv[2]));
								std::cout << "+ " << res.first << " - " << res.second << std::endl;
							}
						}
						else {
							res = sb.show_finance();
							std::cout << "+ " << res.first << " - " << res.second << std::endl;
						}
					}
					else if (argv[0] == "show") {
						auto sql = sb.show();
						if (argv.size() > 2) throw param_exception();
						for (size_t i = 1; i < argv.size(); ++i) {
							auto pr = splitKeyValue(argv[i]);
							if (pr.first == "ISBN")
								sql.limitISBN(pr.second);
							else if (pr.first == "name")
								sql.limitName(pr.second);
							else if (pr.first == "author")
								sql.limitAuthor(pr.second);
							else if (pr.first == "keyword")
								sql.limitKey(pr.second);
						}
						sql.for_each(printBook, printBookNothing);
					}
					else if (argv[0] == "buy") {
						if (argv.size() != 3) throw param_exception();
						if (argv[2].find_first_not_of("0123456789") != std::string::npos)
							throw param_exception();
						double cost = sb.buy(argv[1], std::stoi(argv[2]));
						std::cout << cost << std::endl;
					}
					else if (argv[0] == "select") {
						if (argv.size() != 2) throw param_exception();
						sb.select(argv[1]);
					}
					else if (argv[0] == "modify") {
						BookModify bm;
						for (size_t i = 1; i < argv.size(); ++i) {
							auto pr = splitKeyValue(argv[i]);
							if (pr.first == "ISBN")
								bm.modifyISBN(pr.second);
							else if (pr.first == "name")
								bm.modifyName(pr.second);
							else if (pr.first == "author")
								bm.modifyAuthor(pr.second);
							else if (pr.first == "keyword")
								bm.modifyKey(pr.second);
							else if (pr.first == "price") {
								if (pr.second.find_first_not_of("0123456789.") != std::string::npos)
									throw book_exception("-key=value param wrong");
								bm.modifyPrice(std::stod(pr.second));
							}
						}
						sb.modify(bm);
					}
					else if (argv[0] == "import") {
						if (argv.size() != 3) throw param_exception();
						if (argv[1].find_first_not_of("0123456789") != std::string::npos)
							throw param_exception();
						if (argv[2].find_first_not_of("0123456789.") != std::string::npos)
							throw param_exception();
						sb.Import(std::stoi(argv[1]), std::stod(argv[2]));
					}
					else if (argv[0] == "quit" || argv[0] == "exit")
						break;
					else if (argv[0] == "log" && argv.size() == 1) {
					}
					else if (argv[0] == "who") {
						std::cout << sb.st.back().first.id << ' ' << int(sb.st.back().first.privilege) << std::endl;
					}
					else
						throw param_exception();
				} catch (book_exception const &be) {
					std::cout << "Invalid\n";
					// std::cerr << be.what() << std::endl;
				}
			}
		} catch (std::exception const &e) {
			std::cerr << e.what() << std::endl;
		} catch (...) {
			std::cerr << "What happened ?? " << std::endl;
		}
	}
	return 0;
}
