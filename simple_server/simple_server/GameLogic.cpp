#include "GameLogic.h"
#include <iostream>

using namespace std;
using namespace chrono;

std::time_t GameLogic::m_current_time;
shared_ptr<GameLogic> GameLogic::m_gameLogic;

GameLogic::GameLogic(boost::asio::io_service& service) : timer_one_second_(service, boost::posix_time::seconds(1)),
                                                         timer_one_minute_(service, boost::posix_time::minutes(1)),
                                                         timer_one_hour_(service, boost::posix_time::hours(1)),
                                                         timer_24_hour_(service, boost::posix_time::hours(24))
{
	auto timeinfo = tm();
	timeinfo.tm_year = 115;
	timeinfo.tm_mon = 8;      // month: january
	timeinfo.tm_mday = 30;     // day: 1st
	auto tt = mktime(&timeinfo);

	m_start_clock = system_clock::from_time_t(tt);
	judgeTime();
}

GameLogic::~GameLogic() {
}

shared_ptr<GameLogic> GameLogic::getLogicInstance() {
	return m_gameLogic;
}

void GameLogic::init(boost::asio::io_service& service) {
	if (!m_gameLogic) {
		m_gameLogic = shared_ptr<GameLogic>(new GameLogic(service));
	}
}

void GameLogic::start() {
	cout << "引用指针的数量: " << m_gameLogic.use_count() << endl;

	timer_one_second_.async_wait(std::bind(&GameLogic::oneSec, shared_from_this(), placeholders::_1));
	timer_one_minute_.async_wait(std::bind(&GameLogic::oneMin, shared_from_this(), placeholders::_1));
	timer_one_hour_.async_wait(std::bind(&GameLogic::oneHour, shared_from_this(), placeholders::_1));
	timer_24_hour_.async_wait(std::bind(&GameLogic::twentyFourHour, shared_from_this(), placeholders::_1));
}

void GameLogic::oneMin(const boost::system::error_code&) {
	timer_one_minute_.expires_at(timer_one_minute_.expires_at() + boost::posix_time::minutes(1));
	timer_one_minute_.async_wait(std::bind(&GameLogic::oneMin, shared_from_this(), placeholders::_1));
}

void GameLogic::oneHour(const boost::system::error_code&) {
	judgeTime();

	timer_one_hour_.expires_at(timer_one_hour_.expires_at() + boost::posix_time::hours(1));
	timer_one_hour_.async_wait(std::bind(&GameLogic::oneHour, shared_from_this(), placeholders::_1));
}

void GameLogic::twentyFourHour(const boost::system::error_code&) {
	timer_24_hour_.expires_at(timer_24_hour_.expires_at() + boost::posix_time::hours(24));
	timer_24_hour_.async_wait(std::bind(&GameLogic::twentyFourHour, shared_from_this(), placeholders::_1));
}

void GameLogic::oneSec(const boost::system::error_code&) {
	m_current_clock = system_clock::now();
	m_current_time = system_clock::to_time_t(m_current_clock);

	timer_one_second_.expires_at(timer_one_second_.expires_at() + boost::posix_time::seconds(1));
	timer_one_second_.async_wait(std::bind(&GameLogic::oneSec, shared_from_this(), placeholders::_1));
}

void GameLogic::judgeTime() {
	auto d = system_clock::now() - m_start_clock;

	m_currentDay = duration_cast<days_type> (d);
	m_currentWeek = duration_cast<weeks_type> (d);
	m_currentMonth = duration_cast<months_type> (d);
}