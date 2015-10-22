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
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("58.67.194.141"), ServerPort);
		/*
		auto conn = std::make_shared<Connection>(io_service);
		conn->start(ep, "0");

		auto conn2 = std::make_shared<Connection>(io_service);
		conn2->start(ep, "1");

		io_service.run();
		*/

		std::vector<std::shared_ptr<Connection>> connVec;

		for (auto i = 0; i < 20; ++i) {
			std::this_thread::sleep_for(std::chrono::seconds(1));			
			auto conn = std::make_shared<Connection>(io_service);
			char name[255];
			conn->setName(_itoa(i, name, 10));
			connVec.push_back(conn);					
		}

		for (size_t i = 0; i < connVec.size(); ++i) {			
			connVec.at(i)->start(ep);
		}

		io_service.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
