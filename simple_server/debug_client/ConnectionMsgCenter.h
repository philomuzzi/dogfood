#pragma once

#include <map>
#include <memory>
#include "../Utility/singleton.h"

class Connection;
using ConnectionMsgCallback = bool (*)(std::shared_ptr<Connection> player, const void *msg, const short msglen);
class ConnectionMsgCenter : public SingletonBase<ConnectionMsgCenter>
{
	ConnectionMsgCenter() = default;
	friend SingletonBase<ConnectionMsgCenter>;

	void registry(short msgid, ConnectionMsgCallback f) {
		m_mm[msgid] = f;
	}

public:
	~ConnectionMsgCenter() = default;
	bool registry();
	bool dispatch(short msgid, std::shared_ptr<Connection> player, const void *msg, const short msglen) {
		auto it = m_mm.find(msgid);
		return it == m_mm.end() ? false : (it->second)(player, msg, msglen);
	}

private:
	std::map<short, ConnectionMsgCallback> m_mm;
};