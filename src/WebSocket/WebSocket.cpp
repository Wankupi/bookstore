#include "WebSocket.h"
#include <cstring>
#include <netinet/in.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <string>
#include <sstream>
#include <sys/epoll.h>
#include <iostream>

std::string base64encode(unsigned char const *src, size_t n) {
	EVP_ENCODE_CTX *pCtx = EVP_ENCODE_CTX_new();
	EVP_EncodeInit(pCtx);
	unsigned char *buff = new unsigned char[2 * n];
	int n_res = 0;
	int count = 0;
	int ret = EVP_EncodeUpdate(pCtx, buff, &count, reinterpret_cast<unsigned char const *>(src), n);
	n_res += count;
	EVP_EncodeFinal(pCtx, buff + n_res, &count);
	n_res += count;
	EVP_ENCODE_CTX_free(pCtx);
	std::string res{reinterpret_cast<char *>(buff)};
	delete[] buff;
	res.pop_back();
	return res;
}

unsigned long long htonll(unsigned long long x) {
	unsigned int a1 = x >> 32, a2 = x;
	a1 = htonl(a1);
	a2 = htonl(a2);
	return (static_cast<unsigned long long>(a2) << 32) | a1;
}

WebSocket::WebSocket() : server_fd{0} {}

int WebSocket::bind(const char *ip_addr, int port) {
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) return -1;
	int on = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	int bind_res = ::bind(server_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
	if (bind_res != 0) return -1;
	return 0;
}

void WebSocketHandshake(int fd) {
	int const max_len = 1024;
	char buff[max_len];

	recv(fd, buff, max_len, 0);
	std::istringstream stream{buff};
	std::string req_type;
	std::getline(stream, req_type);

	std::map<std::string, std::string> keys;
	std::string line;
	while (std::getline(stream, line)) {
		if (line.empty()) break;
		if (line.back() == '\n' || line.back() == '\r')
			line.pop_back();
		int pos = line.find(": ");
		if (pos == std::string::npos) break;
		auto key = line.substr(0, pos);
		auto value = line.substr(pos + 2);
		keys.emplace(key, value);
	}
	const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	auto websocketKey = keys["Sec-WebSocket-Key"];
	auto serverKey = websocketKey + magicKey;
	unsigned char shaHash[20];
	SHA1(reinterpret_cast<unsigned char const *>(serverKey.c_str()), serverKey.size(), shaHash);
	auto serverKeyRet = base64encode(shaHash, 20);

	std::string response = "HTTP/1.1 101 Switching Protocols\r\n";
	response += "Upgrade: websocket\r\n";
	response += "Connection: upgrade\r\n";
	response += "Sec-WebSocket-Accept: " + serverKeyRet + "\r\n";
	response += "\r\n";

	send(fd, response.c_str(), response.size(), MSG_DONTWAIT);
}

struct WebSocketHeader {
	int opmode;
	bool is_mask;
	unsigned long long len;
	unsigned char mask[4];
};

int read_header(char const *buff, WebSocketHeader &header) {
	char const *buff_ = buff;
	unsigned char a = *buff++;
	header.opmode = a & 0xf;
	a = *buff++;
	header.is_mask = a & 0x80;
	header.len = a & 0x7f;
	if (header.len == 126) {
		header.len = *reinterpret_cast<unsigned short const *>(buff);
		header.len = htons(header.len);
		buff += 2;
	}
	else if (header.len == 127) {
		header.len = *reinterpret_cast<unsigned long long const *>(buff);
		header.len = htonll(header.len);
		buff += 8;
	}
	if (header.is_mask) {
		for (int i = 0; i < 4; ++i)
			header.mask[i] = *buff++;
	}
	return buff - buff_;
}

int recv_data(int fd, std::string &data) {
	int const max_len = 1048 * 10;
	std::string buff;
	buff.resize(max_len);
	int n = recv(fd, &buff[0], buff.size(), 0);
	if (n <= 0)
		return -1;
	WebSocketHeader header{};
	int rd = read_header(&buff[0], header);
	if (header.opmode == 8)
		return -1;
	data.resize(header.len);
	for (int i = 0; i < header.len; ++i)
		data[i] = buff[rd + i] ^ header.mask[i & 3];
	return 0;
}

void WebSocket::start() {
	int listen_res = listen(server_fd, 32767);
	if (listen_res != 0) throw std::exception();

	epoll_event event;
	std::vector<epoll_event> events;
	events.resize(1024);
	int epfd = epoll_create1(0);
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = server_fd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event);

	while (true) {
		int n = epoll_wait(epfd, events.data(), events.size(), -1);
		if (n < 0) break;
		for (int i = 0; i < n; ++i) {
			auto &ev = events[i];
			if (ev.events & EPOLLIN) {
				if (ev.data.fd == server_fd) {
					sockaddr addr{};
					socklen_t len{};
					int fd = accept(server_fd, &addr, &len);
					WebSocketHandshake(fd);
					if (on_connect) on_connect(*this, fd);
					event.data.fd = fd;
					epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
				}
				else {
					std::string msg;
					int res = recv_data(ev.data.fd, msg);
					if (res == 0 && on_message)
						on_message(*this, ev.data.fd, msg);
					else this->close(ev.data.fd);
				}
			}
		}
	}
}

int write_header(char *buff, int len) {
	char *buff_ = buff;
	*buff++ = 0x81;
	if (len < 126)
		*buff++ = len;
	else if (len <= 32767) {
		*buff++ = 126;
		*reinterpret_cast<unsigned short *>(buff) = htons(len);
		buff += 2;
	}
	else {
		*buff++ = 127;
		*reinterpret_cast<unsigned long long *>(buff) = htonll(len);
		buff += 8;
	}
	return buff - buff_;
}

void WebSocket::send(int fd, std::string const &data) {
	unsigned long long len = data.size();
	std::string buff;
	buff.resize(data.size() + 10);
	int wd = write_header(&buff[0], data.size());
	for (int i = 0; i < len; ++i)
		buff[wd + i] = data[i];
	::send(fd, &buff[0], wd + len, MSG_DONTWAIT);
}

void WebSocket::close(int fd) {
	if (on_close) on_close(*this, fd);
	::close(fd);
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
}
