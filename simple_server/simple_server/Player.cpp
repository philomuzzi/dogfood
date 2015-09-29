#include "Player.h"
#include <iostream>
#include "TableManager.h"
#include "ClientCommand/login_msg.pb.h"

using namespace std;

GamePlayer::~GamePlayer()
{
	cout << "Destruct GamePlayer" << endl;
}

void GamePlayer::online(const network::command::Player& pb_player) {
	m_player.CopyFrom(pb_player);

	initNewPlayer();
}

void GamePlayer::initNewPlayer() {
	auto ptr = TableManager::getInstance().getTable("PlayerInit");
	int airplaneid = ptr->asInt(0, "airplane_id");
	int pilotid = ptr->asInt(0, "pilot_id");
	int diamond = ptr->asInt(0, "diamond");
	int gold = ptr->asInt(0, "gold");
	int energy = ptr->asInt(0, "energy");
	int bag_size = ptr->asInt(0, "bag_size");
	int pet1_id = ptr->asInt(0, "pet1_id");
	int pet2_id = ptr->asInt(0, "pet2_id");

	m_player.set_gold(gold);
	m_player.set_diamond(diamond);
	m_player.set_energy(energy);
	m_player.set_level(1);
	m_player.set_exp(0);

	//Bag *bag = m_player.mutable_bag();
	//bag->set_size(bag_size);

	//DEBUG("%d, %d, %d, %d, %d", airplaneid, pilotid, diamond, gold, energy);

	//Airplane *air = m_player.add_airplanelist();
	//air->set_id(airplaneid);
	//m_player.set_currentairplane(airplaneid);
	//int position_num = PlanePropertyManager::getInstance().asUInt(airplaneid, "position_number");
	//if (position_num != IMPOSSIBLE_RETURN)
	//{
	//	switch (position_num)
	//	{
	//	case 5:
	//		air->mutable_slotfive()->set_isopen(true);
	//	case 4:
	//		air->mutable_slotfour()->set_isopen(true);
	//	case 3:
	//		air->mutable_slotthree()->set_isopen(true);
	//	case 2:
	//		air->mutable_slottwo()->set_isopen(true);
	//	case 1:
	//		air->mutable_slotone()->set_isopen(true);
	//	default:
	//		break;
	//	}
	//}

	//Pilot *pilot = m_player.add_pilotlist();
	//pilot->set_id(pilotid);
	//pilot->set_skill_property_id(PilotPropertyManager::getInstance().asUInt(pilotid, "skill_property_id"));
	//m_player.set_currentpilot(pilotid);

	////为了测试给玩家100个扫荡券，后期可以删掉，从掉落表获取
	//m_player.set_wipestock(1000);
	////为了测试给玩家100个闯关道具，后期可以删掉，从掉落表获取
	//m_player.set_itemnum(1000);

	//addItemToBag(pet1_id);
	//addItemToBag(pet2_id);

	//// DEBUG, 所有的玩家互为好友
	//struct AddFriendPlayerAllExec : public Callback<GamePlayer>
	//{
	//	GamePlayer *new_player;
	//	AddFriendPlayerAllExec(GamePlayer *player) : new_player(player)
	//	{
	//	}
	//	bool exec(GamePlayer *entry)
	//	{
	//		if (strlen(entry->name) != 0 && entry->name != new_player->name)
	//		{
	//			new_player->m_player.add_friendlist(entry->name);
	//			entry->m_player.add_friendlist(new_player->name);
	//			DEBUG("New Player: %s, add friend: %s", new_player->name, entry->name);
	//		}
	//		return true;
	//	}
	//}exec(this);

	//GamePlayerManager::getInstance().execEveryPlayer(exec);
}

void GamePlayer::sendPlayerInfo() {
	network::command::PlayerInfo_S playerMsg;
	::network::command::Player* player = playerMsg.mutable_data();
	player->CopyFrom(m_player);
	ConstructMsgMacro(network::command::CMSGPlayerInfo_S, playerMsg);
	m_connection->sendCmdMsg(playerMsg__, playerMsg_size__);
}