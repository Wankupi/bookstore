#pragma once
#ifndef BOOKSTORE_BOOKSTORELEXER_H
#define BOOKSTORE_BOOKSTORELEXER_H

#include "../store/StoreBranch.h"

class BookStoreLexer {
public:
	BookStoreLexer(StoreBranch &store_branch) : store(store_branch) {}
	int processLine(std::string cmd);
private:
	void func_su(std::vector<std::string> const &argv);
	void func_logout(std::vector<std::string> const &argv);
	void func_register(std::vector<std::string> const &argv);
	void func_passwd(std::vector<std::string> const &argv);
	void func_useradd(std::vector<std::string> const &argv);
	void func_delete(std::vector<std::string> const &argv);
	void func_show(std::vector<std::string> const &argv);
	void func_buy(std::vector<std::string> const &argv);
	void func_select(std::vector<std::string> const &argv);
	void func_modify(std::vector<std::string> const &argv);
	void func_import(std::vector<std::string> const &argv);
	void func_show_finance(std::vector<std::string> const &argv);
	void func_log(std::vector<std::string> const &argv);
private:
	StoreBranch &store;
};

#endif // BOOKSTORE_BOOKSTORELEXER_H
