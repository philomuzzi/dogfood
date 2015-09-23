#pragma once
#include <boost/asio.hpp>

class Server {
public:
	Server(boost::asio::io_service& io_service, short port)
		: acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		  , socket_(io_service) {
		do_accept();
	}

	~Server();

private:
	void do_accept();

	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket socket_;
};
