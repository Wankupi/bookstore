#include "InstanceDaemon.h"
#include "lexer/BookStoreLexer.h"
#include "store/StoreBranch.h"
#include <iostream>

int main() {
	InstanceDaemon instanceDaemon("running.status");
	if (!instanceDaemon.status()) {
		std::cout << "Failed to start, another program instance may be running." << std::endl
				  << "Or you could delete the file 'running.status'." << std::endl;
	}
	else {
		std::cout << std::fixed << std::setprecision(2);
		Users users("data/users/users.db", "data/users/users.idx");
		Books books("data/book/books.db", "data/book/ISBN.idx", "data/book/Name.idx", "data/book/author.idx", "data/book/key.idx");
		Finance finance("data/pay.log");
		StoreBranch sb(users, books, finance);
		BookStoreLexer lexer(sb);
		std::string cmd;

		while (std::getline(std::cin, cmd)) {
			try {
				int r = lexer.processLine(std::move(cmd));
				if (r) break;
			} catch (...) {
				std::cout << "Invalid\n";
			}
		}
	}
	return 0;
}
