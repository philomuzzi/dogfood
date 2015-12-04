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

		std::vector<std::shared_ptr<Connection>> connVec;

		for (auto i = 0; i < 50; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			auto conn = std::make_shared<Connection>(io_service);
			char name[255];
			conn->setName(_itoa(i, name, 10));
			connVec.push_back(conn);
		}

		for (size_t i = 0; i < connVec.size(); ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			connVec.at(i)->start(ep);
		}

		std::thread t([&io_service, &ep]() {
			io_service.run();
		});

		t.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
