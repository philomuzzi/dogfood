#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"
#include "ClientMsgCenter.h"
#include "ConnectionMsgCenter.h"

int main(int argc, char* argv[]) {
	try {
		//if (argc != 2) {
		//	std::cerr << "Usage: async_tcp_echo_server <port>\n";
		//	return 1;
		//}

		// 不要数据库服务，每次都是新建帐号
		// 可以进入和退出游戏
		// 没有结算
		// 可以读取csv表格



		ClientMsgCenter::getInstance().registry();
		ConnectionMsgCenter::getInstance().registry();

		boost::asio::io_service io_service;
		Server s(io_service, 8001);
		io_service.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}