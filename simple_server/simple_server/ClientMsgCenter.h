#pragma once

#include "Utility/singleton.h"
#include "Game_Define.h"
#include <map>
#include <memory>

class GamePlayer;
using ClientMsgCallback = bool (*)(std::shared_ptr<GamePlayer> player, const void *msg, const uint16 msglen);
class ClientMsgCenter : public SingletonBase<ClientMsgCenter>
{
	ClientMsgCenter() = default;	
	friend SingletonBase<ClientMsgCenter>;

	void registry(uint16 msgid, ClientMsgCallback f) {
		m_mm[msgid] = f;
	}

public:
	~ClientMsgCenter() = default;
	bool registry();
	bool dispatch(uint16 msgid, std::shared_ptr<GamePlayer> player, const void *msg, const uint16 msglen) {
		auto it = m_mm.find(msgid);
		return it == m_mm.end() ? false : (it->second)(player, msg, msglen);
	}

private:
	std::map<uint16, ClientMsgCallback> m_mm;
};

