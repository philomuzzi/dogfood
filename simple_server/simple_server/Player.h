#pragma once

#include <memory>
#include <boost/asio.hpp>

#define HeadLength		4
#define MSGIDPACKET		0x7fff0000
#define MSGLENPACKET	0x0000ffff


class Player : public std::enable_shared_from_this<Player> {
public:
	explicit Player(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}

	void start();
	void write(std::string data, std::size_t length);

private:
	void do_read();

	boost::asio::ip::tcp::socket socket_;
	enum {max_length = 65535};
	char read_buffer_[max_length];
	std::array<char, max_length> write_buffer_;

	int already_read_{ 0 };

	int id_;
	static int global_id_;

public:
	int id1() const {
		return id_;
	}
};