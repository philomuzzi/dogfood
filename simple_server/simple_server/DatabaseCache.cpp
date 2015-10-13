#include "DatabaseCache.h"
#include "DatabaseConnection.h"


DatabaseCache::DatabaseCache()
{
}


DatabaseCache::~DatabaseCache()
{
}

network::command::Player DatabaseCache::getPlayer(const std::string accid) {
	if (m_playercache_map.end() == m_playercache_map.find(accid)) {
		network::command::Player player;
		DatabaseConnection::getInstance().getPlayerByAccount(accid, player);
		m_playercache_map.insert(std::make_pair(accid, player));
	}

	return m_playercache_map.at(accid);
}