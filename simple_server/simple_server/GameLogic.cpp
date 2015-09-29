#include "GameLogic.h"
#include <boost\function.hpp>
#include <iostream>

using namespace std;
using namespace chrono;

system_clock::time_point GameLogic::m_current_time;
shared_ptr<GameLogic> GameLogic::gameLogic_;

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
	cout << "引用指针的数量: " << gameLogic_.use_count() << endl;

	timer_one_second_.async_wait(std::bind(&GameLogic::oneSec, shared_from_this(), placeholders::_1));
	timer_one_minute_.async_wait(std::bind(&GameLogic::oneMin, shared_from_this(), placeholders::_1));
	timer_one_hour_.async_wait(std::bind(&GameLogic::oneHour, shared_from_this(), placeholders::_1));
	timer_24_hour_.async_wait(std::bind(&GameLogic::twentyFourHour, shared_from_this(), placeholders::_1));
}

void GameLogic::oneMin(const boost::system::error_code&) {
	cout << "one Min timer" << endl;
	auto tp = std::chrono::system_clock::now();
	std::time_t cur_time = std::chrono::system_clock::to_time_t(tp);
	std::string str_time = std::ctime(&cur_time);
	std::cout << "One Min Time: " << str_time;

	cout << "引用指针的数量: " << gameLogic_.use_count() << endl;

	timer_one_minute_.expires_at(timer_one_minute_.expires_at() + boost::posix_time::minutes(1));

	timer_one_minute_.async_wait(std::bind(&GameLogic::oneMin, shared_from_this(), placeholders::_1));
}

void GameLogic::oneHour(const boost::system::error_code&) {
	cout << "one Hour timer" << endl;
	auto tp = std::chrono::system_clock::now();
	std::time_t cur_time = std::chrono::system_clock::to_time_t(tp);
	std::string str_time = std::ctime(&cur_time);
	std::cout << "One Hour Time: " << str_time;

	cout << "引用指针的数量: " << gameLogic_.use_count() << endl;

	timer_one_hour_.expires_at(timer_one_hour_.expires_at() + boost::posix_time::hours(1));

	timer_one_hour_.async_wait(std::bind(&GameLogic::oneHour, shared_from_this(), placeholders::_1));
}

void GameLogic::twentyFourHour(const boost::system::error_code&) {
	cout << "24 Hours timer" << endl;
	auto tp = std::chrono::system_clock::now();
	std::time_t cur_time = std::chrono::system_clock::to_time_t(tp);
	std::string str_time = std::ctime(&cur_time);
	std::cout << "24 Hours Time: " << str_time;

	cout << "引用指针的数量: " << gameLogic_.use_count() << endl;

	timer_24_hour_.expires_at(timer_24_hour_.expires_at() + boost::posix_time::hours(24));

	timer_24_hour_.async_wait(std::bind(&GameLogic::twentyFourHour, shared_from_this(), placeholders::_1));
}

void GameLogic::oneSec(const boost::system::error_code&) {
	cout << "one Sec timer" << endl;
	auto tp = std::chrono::system_clock::now();
	std::time_t cur_time = std::chrono::system_clock::to_time_t(tp);
	std::string str_time = std::ctime(&cur_time);

	cout << "引用指针的数量: " << gameLogic_.use_count() << endl;

	timer_one_second_.expires_at(timer_one_second_.expires_at() + boost::posix_time::seconds(1));

	timer_one_second_.async_wait(std::bind(&GameLogic::oneSec, shared_from_this(), placeholders::_1));
}
