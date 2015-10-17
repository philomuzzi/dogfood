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

		char name[255];
		std::vector<std::shared_ptr<std::thread>> thread_pool;
		for (auto i = 0; i < 1000; ++i) {
			std::this_thread::sleep_for(std::chrono::seconds(3));
			std::shared_ptr<std::thread> t = std::make_shared<std::thread>([&] {
				auto conn = std::make_shared<Connection>(io_service);
				conn->start(ep, _itoa(i, name, 10));

				io_service.run();
			});

			thread_pool.push_back(t);
		}
		
		io_service.stop();
		for (auto k : thread_pool)
			k->join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
