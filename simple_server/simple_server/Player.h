#pragma once

#include <memory>
#include <boost/asio.hpp>

class Player : public std::enable_shared_from_this<Player> {
public:
	Player(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}

	void start() {
		do_read();
	}

private:
	void do_read();
	void do_write(std::size_t length);


	boost::asio::ip::tcp::socket socket_;
	enum {max_length = 1024};
	char data_[1024];
};

