#pragma once

#include <memory>
#include <boost/asio.hpp>
#include "../Utility/define.h"
#include <save.pb.h>
#include "login_msg.pb.h"

#define TEST_TIMER 30
#include <iostream>

class Connection : public std::enable_shared_from_this<Connection>, boost::noncopyable {
public:
	explicit Connection(boost::asio::io_service& io) : socket_(io), 
		timer_one_second_(io, boost::posix_time::seconds(TEST_TIMER))
	{
	}

	~Connection();

	void start(boost::asio::ip::tcp::endpoint ep);
	void sendCmdMsg(const char* data, int head);
	void do_connectToGS(network::command::ServerInfo_S rev);
	void startGSLogic();

private:
	void start_game();
	void end_game();
	void do_enterGame(network::command::ServerInfo_S rev);
	void stop();
	void do_connectToPL();
	void do_connection(boost::asio::ip::tcp::endpoint ep);
	void do_read();
	void oneSec(const boost::system::error_code&);

	void displayErr(boost::system::error_code &ec);

	boost::asio::ip::tcp::socket socket_;
	char read_buffer_[Max_DataBufferSize];
	std::array<char, Max_DataBufferSize> write_buffer_;

	int already_read_{0};

	std::string m_name;
	bool m_startLoop { false };
	bool m_startGame{ false };

	network::command::Player m_player;

	using Timer = boost::asio::deadline_timer;
	Timer timer_one_second_;

public:
	void setPlayer(const network::command::Player& player) {
		m_player.CopyFrom(player);
	}

	network::command::Player getPlayer() const {
		return m_player;
	}

	std::string getName() const {
		return m_name;
	}

	void setName(const std::string name) {
		m_name = name;
		std::cout << "set name: " << name << std::endl;
	}

	void setStartGame(bool start_game) {
		m_startGame = start_game;
	}
};
