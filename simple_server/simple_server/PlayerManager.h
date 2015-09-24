#pragma once

#include <map>
#include <memory>

#include "Utility/singleton.h"

class Player;

class PlayerManager : public SingletonBase<PlayerManager>
{
	PlayerManager() = default;
	~PlayerManager() = default;
	friend SingletonBase<PlayerManager>;

public:
	void addUnique(std::shared_ptr<Player>);
	void delUnique(const int id);

private:
	std::map<int, std::shared_ptr<Player>> m_onlinePlayer;
};