#pragma once
#ifndef BOOKSTORE_BASE_H
#define BOOKSTORE_BASE_H

#include <array>
#include <cstring>
#include <iostream>
#include <string>
#include <mutex>

template <std::size_t N>
struct String : public std::array<char, N> {
	using std::array<char, N>::array;
	String(std::string const &str) : String() {
		memcpy(this->data(), str.data(), std::min(N, str.size()));
	}
	String(const char *str) : String() {
		memcpy(this->data(), str, std::min(N, strlen(str)));
	}
	explicit operator std::string() const {
		if (this->back()) {
			std::string s{N};
			memcpy(s.data(), this->data(), N);
		}
		else return std::string(this->data());
	}
	bool allzero() const {
		for (int i = 0; i < N; ++i) if ((*this)[i]) return false;
		return true;
	}
};

template <std::size_t N>
std::ostream &operator<<(std::ostream &os, String<N> const &str) {
	if (str.back())
		os.write(str.data(), N);
	else
		os << str.data();
	return os;
}

struct book_exception : std::exception {
	const char * what() const noexcept override { return "Invalid.\n"; }
};
#endif // BOOKSTORE_BASE_H
