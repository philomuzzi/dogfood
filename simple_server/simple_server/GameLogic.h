#pragma once
#include <boost/asio.hpp>
#include <chrono>
#include <memory>

class GameLogic : public std::enable_shared_from_this<GameLogic>
{
public:
	typedef boost::asio::deadline_timer Timer;
	~GameLogic();

	static std::chrono::system_clock::time_point m_current_time;
	static std::shared_ptr<GameLogic> getLogicInstance();
	static void init(boost::asio::io_service &service);
	void start();

	Timer& getOneSecTimer() {
		return timer_one_second_;
	}

private:
	GameLogic(boost::asio::io_service &service);
	
	GameLogic& operator=(GameLogic&) = delete;
	GameLogic(GameLogic&) = delete;

	void oneSec(const boost::system::error_code&);
	void oneMin(const boost::system::error_code&);
	void oneHour(const boost::system::error_code&);
	void twentyFourHour(const boost::system::error_code&);

	Timer timer_one_second_;
	Timer timer_one_minute_;
	
	Timer timer_one_hour_;
	Timer timer_24_hour_;

	static std::shared_ptr<GameLogic> gameLogic_;
};

