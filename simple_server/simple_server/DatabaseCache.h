#pragma once
#include "Utility/singleton.h"
#include <string>
#include <map>
#include "save.pb.h"

class DatabaseCache : public SingletonBase<DatabaseCache>
{
	DatabaseCache();
	~DatabaseCache();
	friend SingletonBase<DatabaseCache>;

	std::map < std::string, network::command::Player > m_playercache_map;

public:
	network::command::Player getPlayer(const std::string accid);
	void setPlayer(network::command::Player);
};

