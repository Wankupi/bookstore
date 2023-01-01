#include "InstanceDaemon.h"
#include "ThreadPool.h"
#include "WebSocket/WebSocket.h"
#include "lexer/BookStoreLexer.h"
#include "store/StoreBranch.h"
#include <iostream>
#include <map>

std::map<int, std::unique_ptr<StoreBranch>> msb;
std::map<int, std::queue<std::string>> tasks;
std::map<int, std::mutex> task_mtx;

void on_message(WebSocket &ws, int fd, std::string msg, ThreadPool &tp) {
	std::unique_lock lk(task_mtx[fd]);
	auto &que = tasks[fd];
	bool is_empty = que.empty();
	que.emplace(std::move(msg));
	lk.unlock();
	if (is_empty)
		tp.append(fd);
}

void do_real_things(int fd, ThreadPool &tp, WebSocket &ws) {
	std::unique_lock lk(task_mtx[fd]);
	if (tasks[fd].empty()) return;
	std::string msg{std::move(tasks[fd].front())};
	tasks[fd].pop();
	lk.unlock();

	std::cerr << "deal command from " << fd << " <" << msg << ">" << std::endl;
	std::istringstream stream(msg);
	std::ostringstream out;
	auto bak = std::cout.rdbuf();
	std::cout.rdbuf(out.rdbuf());
	std::string line;
	auto p = msb.find(fd);
	if (p == msb.end()) return;
	BookStoreLexer lex(*p->second);
	bool to_close = false;
	while (std::getline(stream, line)) {
		try {
			int r = lex.processLine(std::move(line));
			if (r) {
				to_close = true;
				break;
			}
		} catch (...) {
			std::cout << "Invalid\n";
		}
	}
	std::cout << std::flush;
	std::cout.rdbuf(bak);
	auto to_send = out.str();
	if (!to_send.empty())
		ws.send(fd, to_send);
	if (to_close) ws.close(fd);

	lk.lock();
	if (!tasks[fd].empty()) tp.append(fd);
}

int main() {
	//	InstanceDaemon instanceDaemon("running.status");
	//	if (!instanceDaemon.status()) {
	//		std::cout << "Failed to start, another program instance may be running." << std::endl
	//				  << "Or you could delete the file 'running.status'." << std::endl;
	//	}
	//	else {
	std::cout << std::fixed << std::setprecision(2);
	Users users("data/users/users.db", "data/users/users.idx");
	Books books("data/book/books.db", "data/book/ISBN.idx", "data/book/Name.idx", "data/book/author.idx", "data/book/key.idx");
	Finance finance("data/pay.log");

	WebSocket ws;
	ThreadPool tp(std::thread::hardware_concurrency(), [&ws](int fd, ThreadPool &tp) { do_real_things(fd, tp, ws); });

	ws.bind("127.0.0.1", 10000);
	ws.on_connect = [&](WebSocket &, int fd) {
		msb.emplace(fd, std::make_unique<StoreBranch>(users, books, finance));
	};
	ws.on_message = [&tp](WebSocket &ws, int fd, std::string msg) { on_message(ws, fd, std::move(msg), tp); };
	ws.on_close = [&](WebSocket &, int fd) {
		msb.erase(fd);
	};
	std::cout << "ready to start." << std::endl;
	ws.start();
	//	}
	return 0;
}
