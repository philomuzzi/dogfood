#include "Player.h"
#include <iostream>

void Player::do_write(std::size_t length) {
	auto self(shared_from_this());
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(data_, length),
		[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec) {
				do_read();
			}
		}
	);
}

void Player::do_read() {
	auto self(shared_from_this());
	socket_.async_read_some(
		boost::asio::buffer(data_, max_length),
		[this, self](boost::system::error_code ec, std::size_t length) {
			if (!ec) {
				std::string msg(data_, length);
				std::cout << msg;
				do_write(length);
			}
		}
	);
}
