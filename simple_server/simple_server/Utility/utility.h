#pragma once

#include <random>
#include <chrono>

#ifndef ConstructMsgMacro
#define ConstructMsgMacro(msgid, msg) \
	char msg##__[PlayerConnection::Max_UserDataSize + 2] = {0}; \
	assert(msg.SerializePartialToArray(msg##__, sizeof(msg##__))); \
	int msg##_size__ = (msgid << 16) | msg.ByteSize();
#endif

#ifndef SerializeMsgMacro
#define SerializeMsgMacro(msg) \
	char msg##__[PlayerConnection::Max_UserDataSize + 2] = {0}; \
	assert(msg.SerializePartialToArray(msg##__, sizeof(msg##__))); \
	int msg##_size__ = msg.ByteSize();
#endif

namespace Utility {

	inline int randBetween(int min, int max) {
		std::default_random_engine generator(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
		if (min > max) std::swap(min, max);
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(generator);
	}

	inline bool selectByRegion(const int up, const int down) {
		int m_rand;
		if (down < 1) return false;
		if (up < 1) return false;
		if (up > down - 1) return true;
		std::default_random_engine generator(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
		std::uniform_real_distribution<double> distribution(0, 1);
		m_rand = 1 + static_cast<int>((static_cast<double>(down) * distribution(generator)) / (RAND_MAX + 1.0));
		return (m_rand <= up);
	}
}
