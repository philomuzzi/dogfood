#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"

int main(int argc, char* argv[]) {
	try {
		//if (argc != 2) {
		//	std::cerr << "Usage: async_tcp_echo_server <port>\n";
		//	return 1;
		//}

		boost::asio::io_service io_service;

		Server s(io_service, 8001);

		io_service.run();

	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}