#include "GameLogic.h"
#include <boost\bind.hpp>
#include <iostream>

using namespace std;
using namespace chrono;

system_clock::time_point GameLogic::m_current_time;


GameLogic::GameLogic(boost::asio::io_service& service) : timer_one_second_(service, boost::posix_time::seconds(1)),
                                                         timer_one_minute_(service, boost::posix_time::minutes(1)),
                                                         timer_one_hour_(service, boost::posix_time::hours(1)),
                                                         timer_24_hour_(service, boost::posix_time::hours(24))
														 {

}

GameLogic::~GameLogic() {
}

shared_ptr<GameLogic> GameLogic::getLogicInstance() {
	return gameLogic_;
}

void GameLogic::init(boost::asio::io_service& service) {
	if (!gameLogic_) {
		gameLogic_ = shared_ptr<GameLogic>(new GameLogic(service));
	}
}

void GameLogic::start() {
//	timer_one_second_.async_wait();
//	timer_one_minute_.async_wait(boost::bind(&GameLogic::oneMinute, this));
//	timer_one_hour_.async_wait(boost::bind(&GameLogic::oneHour, this));
//	timer_24_hour_.async_wait(boost::bind(&GameLogic::twentyFourHour, this));
}



void GameLogic::oneSec(const boost::system::error_code&) {
	cout << "one Sec timer" << endl;
//	timer_one_second_.async_wait(boost::bind(&GameLogic::oneSec));
}

void GameLogic::oneMinute(const boost::system::error_code&) {
	cout << "one Minute timer" << endl;
//	timer_one_minute_.async_wait(boost::bind(&GameLogic::oneMinute, this));
}

void GameLogic::oneHour(const boost::system::error_code&) {
	cout << "one Hour timer" << endl;
//	timer_one_hour_.async_wait(boost::bind(&GameLogic::oneHour, this));
}

void GameLogic::twentyFourHour(const boost::system::error_code&) {
	cout << "24 Hour timer" << endl;
//	timer_24_hour_.async_wait(boost::bind(&GameLogic::twentyFourHour, this));
}
