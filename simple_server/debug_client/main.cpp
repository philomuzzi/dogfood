#include <iostream>
#include <boost/asio.hpp>
#include "ConnectionMsgCenter.h"
#include "../Utility/define.h"
#include "Connection.h"

int main(int argc, char* argv[]) {
	try {
		//if (argc != 2) {
		//	std::cerr << "Usage: async_tcp_echo_server <port>\n";
		//	return 1;
		//}

		ConnectionMsgCenter::getInstance().registry();

		boost::asio::io_service io_service;
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), ServerPort);
		auto conn = std::make_shared<Connection>(io_service);
		conn->start(ep, "123");

//		GameLogic::init(io_service);
//		GameLogic::getLogicInstance()->start();

		io_service.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}