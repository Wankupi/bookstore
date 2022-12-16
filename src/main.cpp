#include "InstanceDaemon.h"
#include "StoreBranch.h"
#include "base.h"
#include "file/DataBase.h"
#include "user/User.h"
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
InstanceDaemon instanceDaemon("running.status");

std::vector<std::string> splitCommand(std::string &&cmd) {
	std::vector<std::string> r;
	std::istringstream is(std::move(cmd));
	std::string t;
	while (is >> t)
		r.emplace_back(std::move(t));
	return r;
}

int main() {
	if (!instanceDaemon.status()) {
		std::cout << "Failed to start, another program instance may be running." << std::endl
				  << "Or you could delete the file 'running.status'." << std::endl;
	}
	else {
		try {
			Users users("users/users.db", "users/users.idx");
			StoreBranch sb(users);
			auto invalid = []() { std::cout << "Invalid\n"; };
			while (true) {
				std::string cmd;
				std::getline(std::cin, cmd);
				auto argv = splitCommand(std::move(cmd));
				if (argv.empty()) continue;
				try {
					if (argv[0] == "su") {
						if (argv.size() == 2) sb.login(argv[1], {});
						else if (argv.size() == 3) sb.login(argv[1], argv[2]);
						else throw book_exception();
					}
					else if (argv[0] == "logout") {
						if (argv.size() != 1) throw book_exception{};
						sb.logout();
					}
					else if (argv[0] == "register") {
						if (argv.size() != 4) throw book_exception();
						sb.Register(argv[1], argv[2], argv[3]);
					}
					else if (argv[0] == "passwd") {
						if (argv.size() == 3) sb.passwd(argv[1], {}, argv[2]);
						else if (argv.size() == 4) sb.passwd(argv[1], argv[2], argv[3]);
						else throw book_exception();
					}
					else if (argv[0] == "useradd") {
						if (argv.size() != 5) throw book_exception();
						if (argv[3].size() != 1) throw book_exception();
						int p = argv[3][0] - '0';
						if (p != 1 && p != 3) throw book_exception();
						sb.useradd(argv[1], argv[2], Privilege(p), argv[4]);
					}
					else if (argv[0] == "delete") {
						if (argv.size() != 2) throw book_exception();
						sb.userdel(argv[1]);
					}
					else if (argv[0] == "show") {
					}
					else if (argv[0] == "buy") {
					}
					else if (argv[0] == "select") {
					}
					else if (argv[0] == "modify") {
					}
					else if (argv[0] == "import") {
					}
					else if (argv[0] == "quit" || argv[0] == "exit")
						break;
					else if (argv[0] == "show") {
					}
					else if (argv[0] == "log" && argv.size() == 1) {
					}
					else if (argv[0] == "who") {
						std::cout << sb.st.top().first.id << ' ' << int(sb.st.top().first.privilege) << std::endl;
					}
					else
						invalid();
				} catch (book_exception const &be) {
					std::cout << be.what();
				}
			}
		} catch (const char *const str) {
			std::cout << str << std::endl;
		}
	}
	return 0;
}
