#pragma once

#include <map>
#include <memory>

#include "Utility/singleton.h"

class PlayerConnection;

class PlayerManager : public SingletonBase<PlayerManager>
{
	PlayerManager() = default;
	~PlayerManager() = default;
	friend SingletonBase<PlayerManager>;

public:
	void addUnique(std::shared_ptr<PlayerConnection>);
	void delUnique(const int id);

private:
	std::map<int, std::shared_ptr<PlayerConnection>> m_onlinePlayer;
};