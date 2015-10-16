#pragma once

#include <memory>
#include <boost/asio.hpp>
#include <Utility/define.h>

class PlayerConnection : public std::enable_shared_from_this<PlayerConnection> {
public:
	explicit PlayerConnection(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}
	~PlayerConnection();

	void start();
	void write(std::string data, std::size_t length);
	void sendCmdMsg(const char* data, int head);


private:
	void do_read();

	boost::asio::ip::tcp::socket socket_;
	char read_buffer_[Max_DataBufferSize];
	std::array<char, Max_DataBufferSize> write_buffer_;

	int already_read_{ 0 };

	std::string name;

public:
	std::string getName() const {
		return name;
	}

	void setName(const std::string& name) {
		this->name = name;
	}
};