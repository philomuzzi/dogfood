#pragma once

#include <map>
#include <memory>

#include "Utility/singleton.h"

class GamePlayer;

class PlayerManager : public SingletonBase<PlayerManager>
{
	PlayerManager() = default;
	~PlayerManager() = default;
	friend SingletonBase<PlayerManager>;

public:
	void addPlayer(std::shared_ptr<GamePlayer> player);
	std::shared_ptr<GamePlayer> getPlayerByName(const std::string name);
	void removePlayer(std::shared_ptr<GamePlayer> player);

private:
	std::map<std::string, std::shared_ptr<GamePlayer>> m_onlinePlayer;
};