#pragma once
#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include "Game_Define.h"

class GameLogic : public std::enable_shared_from_this<GameLogic>
{
public:
	using Timer = boost::asio::deadline_timer;

	~GameLogic();

	static std::time_t m_current_time;
	static std::shared_ptr<GameLogic> getLogicInstance();
	static void init(boost::asio::io_service &service);
	void start();

	Timer& getOneSecTimer() {
		return timer_one_second_;
	}

	uint32 getWeek() const {
		return m_currentWeek.count();
	}
	uint32 getDay() const {
		return m_currentDay.count();
	}
	uint32 getMonth() const {
		return m_currentMonth.count();
	}

private:
	explicit GameLogic(boost::asio::io_service &service);
	
	GameLogic& operator=(GameLogic&) = delete;
	GameLogic(GameLogic&) = delete;

	void judgeTime();

	void oneSec(const boost::system::error_code&);
	void oneMin(const boost::system::error_code&);
	void oneHour(const boost::system::error_code&);
	void twentyFourHour(const boost::system::error_code&);

	Timer timer_one_second_;
	Timer timer_one_minute_;
	
	Timer timer_one_hour_;
	Timer timer_24_hour_;

	std::chrono::system_clock::time_point m_current_clock;
	std::chrono::system_clock::time_point m_start_clock;

	using days_type = std::chrono::duration<int, std::ratio<60 * 60 * 24>> ;
	using weeks_type = std::chrono::duration<int, std::ratio<60 * 60 * 24 * 7>>;
	using months_type = std::chrono::duration<int, std::ratio<60 * 60 * 24 * 30>>;

	days_type m_currentDay;
	weeks_type m_currentWeek;
	months_type m_currentMonth;

	static std::shared_ptr<GameLogic> m_gameLogic;
	
};

