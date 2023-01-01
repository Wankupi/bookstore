#pragma once
#ifndef BOOKSTORE_THREADPOOL_H
#define BOOKSTORE_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {
public:
	ThreadPool(int n, std::function<void(int, ThreadPool &)> const &func) : func(func) {
		tds.reserve(n);
		for (int i = 0; i < n; ++i)
			tds.emplace_back(&ThreadPool::thread_function, this);
	}

	void thread_function() {
		std::unique_lock ul(mtx);
		for (;;) {
			if (!tasks.empty()) {
				int task = tasks.front();
				tasks.pop();
				ul.unlock();
				func(task, *this);
				ul.lock();
			}
			else if (is_shutdown) break;
			else
				cv.wait(ul);
		}
	}

	void append(int task) {
		std::unique_lock ul(mtx);
		tasks.push(task);
		ul.unlock();
		cv.notify_all();
	}

	~ThreadPool() {
		is_shutdown = true;
		cv.notify_all();
		for (auto &td : tds)
			td.join();
	}
private:
	std::mutex mtx;
	std::condition_variable cv;
	std::queue<int> tasks;
	bool is_shutdown = false;
	std::function<void(int, ThreadPool &)> func;
	std::vector<std::thread> tds;
};

#endif // BOOKSTORE_THREADPOOL_H
