#include "PlayerManager.h"
#include "PlayerConnection.h"
#include <iostream>

using namespace std;

void PlayerManager::addUnique(std::shared_ptr<PlayerConnection> player) {
	m_onlinePlayer.insert(make_pair(player->id1(), player));
	cout << "add new player: " << player->id1() << endl;
}

void PlayerManager::delUnique(const int id) {
	auto it = m_onlinePlayer.find(id);
	if (it != m_onlinePlayer.end()) {
		m_onlinePlayer.erase(it);
		cout << "del player: " << id << endl;
	}		
}