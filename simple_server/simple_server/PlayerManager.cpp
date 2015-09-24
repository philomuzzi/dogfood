#include "PlayerManager.h"
#include "Player.h"
#include <iostream>

using namespace std;

void PlayerManager::addUnique(std::shared_ptr<GamePlayer> player) {
	m_onlinePlayer.insert(make_pair(player->getAccid(), player));
	cout << "add new player: " << player->getAccid() << endl;
}

void PlayerManager::delUnique(string name) {
	auto it = m_onlinePlayer.find(name);
	if (it != m_onlinePlayer.end()) {
		m_onlinePlayer.erase(it);
		cout << "del player: " << name << endl;
	}		
}

shared_ptr<GamePlayer> PlayerManager::getPlayerByName(const std::string name) {
	auto it = m_onlinePlayer.find(name);
	if (it != m_onlinePlayer.end()) {
		return it->second;
	}

	return nullptr;
}

void PlayerManager::removePlayer(std::shared_ptr<GamePlayer> player) {
	if (player) {
		delUnique(player->getAccid());
	}	
}