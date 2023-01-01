#pragma once
#ifndef BOOKSTORE_WEBSOCKET_H
#define BOOKSTORE_WEBSOCKET_H

#include <functional>
#include <string>

class WebSocket {
public:
	WebSocket();
	int bind(const char *addr, int port);
	void start();
	void send(int fd, std::string const &msg);
	void close(int fd);
public:
	std::function<void(WebSocket&, int)> on_connect;
	std::function<void(WebSocket&, int, std::string)> on_message;
	std::function<void(WebSocket&, int)> on_close;

private:
	int server_fd;
	int epfd;
};

#endif
