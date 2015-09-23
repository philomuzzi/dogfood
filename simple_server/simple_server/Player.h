#pragma once

#include <memory>
#include <boost/asio.hpp>

#define HeadLength		4
#define MSGIDPACKET		0x7fff0000
#define MSGLENPACKET	0x0000ffff

class Player : public std::enable_shared_from_this<Player> {
public:
	Player(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}

	void start();

private:
	void do_read();
	void do_write(std::size_t length);

	boost::asio::ip::tcp::socket socket_;
	enum {max_length = 65535};
	std::array<char, max_length> data_;
};

