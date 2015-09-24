#pragma once

#include "Utility/singleton.h"
#include <map>
#include <memory>

class Player;
typedef bool(*ClientMsgCallback)(std::shared_ptr<Player> player, const void *msg, const short msglen);
class ClientMsgCenter : public SingletonBase<ClientMsgCenter>
{
	ClientMsgCenter() = default;	
	friend SingletonBase<ClientMsgCenter>;

	void registry(short msgid, ClientMsgCallback f) {
		m_mm[msgid] = f;
	}

public:
	~ClientMsgCenter() = default;
	bool registry();
	bool dispatch(short msgid, std::shared_ptr<Player> player, const void *msg, const short msglen) {
		auto it = m_mm.find(msgid);
		return it == m_mm.end() ? false : (it->second)(player, msg, msglen);
	}

private:
	std::map<short, ClientMsgCallback> m_mm;
};

