#pragma once

#include <memory>
#include <boost/asio.hpp>
#include "../Utility/define.h"
#include <google/protobuf/stubs/atomicops.h>

namespace network{namespace command{
	class ServerInfo_S;
}}

class Connection : public std::enable_shared_from_this<Connection>, boost::noncopyable {
public:
	explicit Connection(boost::asio::io_service& io) : socket_(io) {
	}

	~Connection();

	void start(boost::asio::ip::tcp::endpoint ep, std::string name);
	void write(std::string data, std::size_t length);
	void sendCmdMsg(const char* data, int head);
	void do_connectToGS(network::command::ServerInfo_S& rev);

private:
	void do_enterGame(network::command::ServerInfo_S& rev);
	void stop();
	void do_connectToPL();
	void do_connection(boost::asio::ip::tcp::endpoint ep);
	void do_read();

	boost::asio::ip::tcp::socket socket_;
	char read_buffer_[Max_DataBufferSize];
	std::array<char, Max_DataBufferSize> write_buffer_;

	int already_read_{0};

	std::string m_name;
};