#include <iostream>
#include <boost/asio.hpp>
#include "ConnectionMsgCenter.h"
#include "../Utility/define.h"
#include "Connection.h"
#include <thread>
#include <cstdlib>

int main() {
	try {
		ConnectionMsgCenter::getInstance().registry();

		boost::asio::io_service io_service;	
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), ServerPort);
		/*
		auto conn = std::make_shared<Connection>(io_service);
		conn->start(ep, "0");

		auto conn2 = std::make_shared<Connection>(io_service);
		conn2->start(ep, "1");

		io_service.run();
		*/
		
		for (auto i = 0; i < 20; ++i) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			char name[255];
			auto conn = std::make_shared<Connection>(io_service);
			conn->start(ep, _itoa(i, name, 10));				
		}

		io_service.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
