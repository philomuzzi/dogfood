#include "Player.h"
#include <iostream>
#include "TableManager.h"
#include "ClientCommand/login_msg.pb.h"
#include "GameLogic.h"
#include "ClientCommand/game.pb.h"
#include "DatabaseConnection.h"
#include "DatabaseCache.h"

using namespace std;
using namespace network::command;

uint32 GamePlayer::m_sequence = 0;
map<string, shared_ptr<map<string, uint32>>> GamePlayer::sm_nextSendMailTime;

void GamePlayer::online(const Player& pb_player) {
	m_player.CopyFrom(pb_player);

	if (m_player.state() == 0) {
		initNewPlayer();
	}

	auto f_it = sm_nextSendMailTime.find(m_player.accid());
	if (f_it == sm_nextSendMailTime.end())
	{
		m_nextSendClientTime = make_shared<map<string, uint32>>();
		sm_nextSendMailTime.insert(make_pair(m_player.accid(), m_nextSendClientTime));
	}
	else
	{
		m_nextSendClientTime = f_it->second;
	}

	sendSystemMail();
	checkQuestActivity();
	checkWeekRanking();
	setLastLoginTime();
	checkCheckIn();
	sendInitInfo();
	loginJustfiy();
	sendPlayerInfo();

	checkRanking();
}

void GamePlayer::initNewPlayer() {
	auto ptr = TableManager::getInstance().getTable("PlayerInit");
	auto airplaneid = ptr->asInt(0, "airplane_id");
	auto pilotid = ptr->asInt(0, "pilot_id");
	auto diamond = ptr->asInt(0, "diamond");
	auto gold = ptr->asInt(0, "gold");
	auto energy = ptr->asInt(0, "energy");
	auto bag_size = ptr->asInt(0, "bag_size");
	auto pet1_id = ptr->asInt(0, "pet1_id");
	auto pet2_id = ptr->asInt(0, "pet2_id");

	m_player.set_gold(gold);
	m_player.set_diamond(diamond);
	m_player.set_energy(energy);
	m_player.set_level(1);
	m_player.set_exp(0);

	auto bag = m_player.mutable_bag();
	bag->set_size(bag_size);

	//DEBUG("%d, %d, %d, %d, %d", airplaneid, pilotid, diamond, gold, energy);

	auto air = m_player.add_airplanelist();
	air->set_id(airplaneid);
	m_player.set_currentairplane(airplaneid);
	uint32 position_num = TableManager::getInstance().getTable("PlaneProperty")->asInt(airplaneid, "position_number");
	if (position_num != IMPOSSIBLE_RETURN) {
		switch (position_num) {
		case 5:
			air->mutable_slotfive()->set_isopen(true);
		case 4:
			air->mutable_slotfour()->set_isopen(true);
		case 3:
			air->mutable_slotthree()->set_isopen(true);
		case 2:
			air->mutable_slottwo()->set_isopen(true);
		case 1:
			air->mutable_slotone()->set_isopen(true);
		default:
			break;
		}
	}

	auto pilot = m_player.add_pilotlist();
	pilot->set_id(pilotid);
	pilot->set_skill_property_id(TableManager::getInstance().getTable("PilotProperty")->asInt(pilotid, "skill_property_id"));
	m_player.set_currentpilot(pilotid);

	//为了测试给玩家100个扫荡券，后期可以删掉，从掉落表获取
	m_player.set_wipestock(1000);
	//为了测试给玩家100个闯关道具，后期可以删掉，从掉落表获取
	m_player.set_itemnum(1000);

	addItemToBag(pet1_id);
	addItemToBag(pet2_id);

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
	auto player = new Player(m_player);
	PlayerInfo_S playerMsg;
	playerMsg.set_allocated_data(player);
	ConstructMsgMacro(network::command::CMSGPlayerInfo_S, playerMsg);
	m_connection->sendCmdMsg(playerMsg__, playerMsg_size__);

	save();
}

uint64 GamePlayer::generateUUID() {
	uint64 value;
	// 最好再减一个epoch
	value = GameLogic::m_current_time;
	value = value << 32;
	value |= (m_player.id() & 0xffffff) << 24;
	value |= m_sequence++ & 0xFF;
	if (m_sequence == 0x100)
		m_sequence = 0;

	return value;
}

void GamePlayer::sendInitInfo() const {
	Game::InitInfo_S msg;
	msg.set_time(GameLogic::m_current_time);
	printf("服务器时间：%llu\n", msg.time());
	ConstructMsgMacro(CMSGInitInfo_S, msg);
	m_connection->sendCmdMsg(msg__, msg_size__);
}

void GamePlayer::checkin(Play::CheckIn_CS& msg) {
	if (m_player.lastcheckday() == GameLogic::getLogicInstance()->getDay()) {
		printf("一天不能签到多次");
		// return ProtocolReturn(msg, Play::CheckIn_CS::FAIL, CMSGCheckIn_CS);
	}

	// 发放奖励
	auto id = DailyRewardID + m_player.checktimes() + 1;

	auto ptr = TableManager::getInstance().getTable("DailyReward");
	uint32 tmpid = ptr->asInt(id, "id");
	if (id != tmpid) {
		printf("每日签到奖励表格配置错误, %d\n", id);
		return ProtocolReturn(msg, Play::CheckIn_CS::UNKNOWN, CMSGCheckIn_CS);
	}

	printf("奖励表格iD, %d\n", id);
	uint32 item_id = ptr->asInt(id, "rewardid");
	uint32 num = ptr->asInt(id, "quantity");
	if (item_id != IMPOSSIBLE_RETURN && num != IMPOSSIBLE_RETURN) {
		addItemToBag(item_id, num);

		uint32 bonus_id = ptr->asInt(id, "bonusid");
		if (bonus_id != IMPOSSIBLE_RETURN) {
			tmpid = ptr->asInt(bonus_id, "id");
			if (tmpid == bonus_id)
				addItemToBag(bonus_id);
			else
				printf("奖励id在表格中不存在, %d, %d\n", tmpid, bonus_id);
		}

		m_player.set_lastcheckday(GameLogic::getLogicInstance()->getDay());
		m_player.set_checktimes(m_player.checktimes() + 1);
		m_player.set_totalchecktimes(m_player.totalchecktimes() + 1);
		m_player.set_lastchecktime(GameLogic::m_current_time);

		printf("签到成功, 第%d次\n", m_player.checktimes());
		msg.set_checktimes(m_player.checktimes());
		sendPlayerInfo();
		return ProtocolReturn(msg, Play::CheckIn_CS::SUCCESS, CMSGCheckIn_CS);
	}

	return ProtocolReturn(msg, Play::CheckIn_CS::UNKNOWN, CMSGCheckIn_CS);
}

void GamePlayer::checkCheckIn() {
	if (GameLogic::getLogicInstance()->getMonth() != m_player.lastloginmonth())
		clearCheckIn();
}

void GamePlayer::clearCheckIn() {
	m_player.set_lastcheckday(0);
	m_player.set_checktimes(0);
}

void GamePlayer::setLastLoginTime() {
	m_player.set_lastloginweek(GameLogic::getLogicInstance()->getWeek());
	m_player.set_lastloginday(GameLogic::getLogicInstance()->getDay());
	m_player.set_lastloginmonth(GameLogic::getLogicInstance()->getMonth());
}

bool GamePlayer::addEnergy(uint32 value) {
	uint32 m_energy;
	m_energy = (m_player.energy() + value);
	m_player.set_energy(m_energy);

	return true;
}

bool GamePlayer::subEnergy(uint32 value) {
	auto m_energy = m_player.energy() > value ? (m_player.energy() - value) : 0;
	m_player.set_energy(m_energy);
	return true;
}

bool GamePlayer::subWipeStock(uint32 value) {
	auto m_value = m_player.wipestock() > value ? m_player.wipestock() - value : 0;
	m_player.set_wipestock(m_value);
	return true;
}

bool GamePlayer::addWipeStock(uint32 value) {
	m_player.set_wipestock(m_player.wipestock() + value);
	return true;
}

bool GamePlayer::subFbUnlockItem(uint32 value) {
	auto m_value = m_player.itemnum() > value ? m_player.wipestock() - value : 0;
	m_player.set_itemnum(m_value);
	return true;
}

bool GamePlayer::addFbUnlockItem(uint32 value) {
	m_player.set_itemnum(m_player.itemnum() + value);
	return true;
}


Pilot* GamePlayer::getPilot(uint32 id) {
	Pilot* pilot = nullptr;
	auto pilotlist = m_player.mutable_pilotlist();
	auto it_pilot = pilotlist->begin();
	for (; it_pilot != pilotlist->end(); ++it_pilot) {
		if (it_pilot->id() == id) {
			pilot = &(*it_pilot);
			break;
		}
	}

	return pilot;
}

Airplane* GamePlayer::getPlane(uint32 id) {
	Airplane* airplane = nullptr;
	auto airlist = m_player.mutable_airplanelist();
	auto it_air = airlist->begin();
	for (; it_air != airlist->end(); ++it_air) {
		if (it_air->id() == id) {
			airplane = &(*it_air);
			break;
		}
	}

	return airplane;
}

void GamePlayer::addExp(const uint32 value) {
	m_player.set_exp(m_player.exp() + value);

	levelUp(value);
}

void GamePlayer::levelUp(uint32 value) {
	uint32 max_level = TableManager::getInstance().getTable("PlayerInit")->asInt(0, "max_level");
	if (m_player.level() == max_level) {
		m_player.set_extraexp(m_player.extraexp() + value);
		return;
	}
	uint32 m_exp;
	m_exp = TableManager::getInstance().getTable("LevelUpExp")->asInt(m_player.level(), "exp");
	while ((m_player.exp() + value) >= m_exp) {
		m_player.set_level(m_player.level() + 1);
		if (m_player.level() == max_level) {
			m_player.set_extraexp(m_player.extraexp() + m_player.exp() + value - m_exp);
			m_player.set_exp(0);
			break;
		}
		m_player.set_exp(m_player.exp() + value - m_exp);
		value -= m_exp;
		m_exp = TableManager::getInstance().getTable("LevelUpExp")->asInt(m_player.level(), "exp");
	}
}

void GamePlayer::save() {
	m_player.set_state(1);
	DatabaseCache::getInstance().setPlayer(m_player);
	DatabaseConnection::getInstance().updatePlayer(m_player);
}

void GamePlayer::sendFriendNextMailTime() const {
	Mail::FriendMailNextTime_S sf_msg;
	auto it = m_nextSendClientTime->begin();
	for (;it != m_nextSendClientTime->end(); ++it)
	{
		if (it->second > GameLogic::m_current_time)
		{
			auto tmp = sf_msg.add_nexttimelist();
			tmp->set_accid(it->first);
			tmp->set_nextsendtime(it->second);
		}
	}

	if (sf_msg.nexttimelist_size())
	{
		ConstructMsgMacro(CMSGFriendMailNextTime_S, sf_msg);
		m_connection->sendCmdMsg(sf_msg__, sf_msg_size__);
	}
}

void GamePlayer::setting(Game::PlayerSetting_CS &msg)
{
	//m_player.set_allowenergymail(msg.allowenergymail());

	//sendPlayerInfo();
	//msg.set_result(Game::PlayerSetting_CS::SUCCESS);
	//ConstructMsgMacro(CMSGPlayerSetting_CS, msg);
	//m_connection->sendCmdMsg(msg__, msg_size__);
}

void GamePlayer::requestUserMail()
{
	Mail_SS msg;
	msg.set_accid(m_player.accid());
//	SerializeMsgMacro(msg);

//	INFO("启动时，向数据库请求用户%s邮件", m_player.accid().c_str());
//	databaseClient->sendClientCmdToDatabase(network::command::CMSGMail_SS, msg__, msg_size__);
}


void GamePlayer::loginJustfiy()
{
	if (m_player.lastofflinetime() == 0)
		return;

	// 这里不精确，但是每小时的固定减少将会补偿回来
	int nhour = (GameLogic::m_current_time - m_player.lastofflinetime()) / ONE_HOUR;
	if (nhour)
	{
		adjustPilotFriendliness(nhour);
	}
}

void GamePlayer::addPilotFriendliness(Pilot *pilot, const uint32 value) const {
	uint32 cap = TableManager::getInstance().getTable("PilotProperty")->asInt(pilot->id(), "friendlinesscap");
	printf("pilot %d 的友好度上限为 %d, add: %d, now: %d\n", pilot->id(), cap, value, pilot->friendliness());
	if (value != IMPOSSIBLE_RETURN)
	{
		if (cap >= pilot->friendliness())
		{
			if (cap > pilot->friendliness() + value)
				pilot->set_friendliness(pilot->friendliness() + value);
			else
				pilot->set_friendliness(cap);
		}
	}
}

void GamePlayer::subPilotFriendliness(Pilot *pilot, const uint32 value) const {
	if (pilot->friendliness() == 0)
		return;
	if (value != IMPOSSIBLE_RETURN)
	{
		if (pilot->friendliness() <= value)
			pilot->set_friendliness(0);
		else
			pilot->set_friendliness(pilot->friendliness() - value);
	}
}

void GamePlayer::adjustPilotFriendliness()
{
	adjustPilotFriendliness(1);
}

void GamePlayer::adjustPilotFriendliness(uint32 hours)
{
	uint32 value = TableManager::getInstance().getTable("PlayerInit")->asInt(0, "friendliness_drop_timer");
	if (value && value != IMPOSSIBLE_RETURN)
	{
		for (int i = 0; i < m_player.pilotlist_size(); ++i)
		{
			Pilot *pilot = m_player.mutable_pilotlist(i);
			if (pilot)
			{
				if (pilot->notdropfriendlinesstime() == 0)
				{
					subPilotFriendliness(pilot, value * hours);
				}
				else if (pilot->notdropfriendlinesstime() >= hours * ONE_HOUR)
				{
					pilot->set_notdropfriendlinesstime(pilot->notdropfriendlinesstime() - hours * ONE_HOUR);
				}
				else
				{
					pilot->set_notdropfriendlinesstime(0);
					hours -= pilot->notdropfriendlinesstime() / ONE_HOUR;
					subPilotFriendliness(pilot, hours * value);
				}
			}
		}
	}
}


void GamePlayer::checkQuestActivity()
{
	if (GameLogic::getLogicInstance()->getDay() > m_player.lastloginday())
		clearQuestActivity();
}

void GamePlayer::clearQuestActivity()
{
	m_player.clear_finisheddailyquestid();
	m_player.clear_dailystatistics();
}

void GamePlayer::updatePilotFriendliness(Game::FriendlinessInteract_CS &msg)
{
	printf("更新用户和飞行员的亲密度\n");

	Pilot *pilot = getPilot(msg.pilotid());
	if (pilot == nullptr)
	{
		printf("传入的飞行员ID错误, %d\n", msg.pilotid());
		return ProtocolReturn(msg, Game::FriendlinessInteract_CS::UNKNOWN, CMSGFriendlinessInteract_CS);
	}

	uint32 id = TableManager::getInstance().getTable("InteractDialog")->asInt(msg.interactid(), "id");
	if (id != msg.interactid())
	{
		printf("传入的交互ID错误, %d\n", id);
		return ProtocolReturn(msg, Game::FriendlinessInteract_CS::UNKNOWN, CMSGFriendlinessInteract_CS);
	}

	if (msg.type() == Game::FriendlinessInteract_CS::Add)
	{
		uint32 value = TableManager::getInstance().getTable("InteractDialog")->asInt(msg.interactid(), "addfriendliness");
		addPilotFriendliness(pilot, value);
	}
	else
	{
		uint32 value = TableManager::getInstance().getTable("InteractDialog")->asInt(msg.interactid(), "reducefriendliness");
		subPilotFriendliness(pilot, value);
	}

	msg.set_currentfriendliness(pilot->friendliness());
	return ProtocolReturn(msg, Game::FriendlinessInteract_CS::SUCCESS, CMSGFriendlinessInteract_CS);
}

void GamePlayer::itemForFriendliness(Game::ItemForFriendliness_CS &msg)
{
	printf("用户亲密度道具交互\n");

	Pilot *pilot = getPilot(msg.pilotid());
	if (pilot == nullptr)
	{
		printf("传入的飞行员ID错误\n");
		return ProtocolReturn(msg, Game::ItemForFriendliness_CS::UNKNOWN, CMSGItemForFriendliness_CS);
	}

	auto ptr = TableManager::getInstance().getTable("PlayerItem");
	uint32 type = ptr->asInt(msg.itemid(), "type");
	uint32 subtype = ptr->asInt(msg.itemid(), "subtype");
	if (type == static_cast<int>(PlayerItemType::Item_Interact))
	{
		uint32 exp = ptr->asInt(msg.itemid(), "exp");
		if (subtype == static_cast<int>(InteractItemSubType::Interact_Add))
		{
			addPilotFriendliness(pilot, exp);
		}
		else if (subtype == static_cast<int>(InteractItemSubType::Interact_Stop))
		{
			pilot->set_notdropfriendlinesstime(pilot->notdropfriendlinesstime() + exp);
		}
		else if (subtype == static_cast<int>(InteractItemSubType::Interact_Random))
		{
			uint32 interact_prop = ptr->asInt(msg.itemid(), "interact_prop");
			uint32 rand = Utility::randBetween(0, 100);
			if (rand <= interact_prop)
			{
				addPilotFriendliness(pilot, exp);
			}
			else
			{
				uint32 interact_sub_exp = ptr->asInt(msg.itemid(), "interact_sub_exp");

				subPilotFriendliness(pilot, interact_sub_exp);
			}
		}
		else
		{
			printf("互动道具交互，物品信息: id %d, type %d, subtype %d\n", msg.itemid(), type, subtype);
			return ProtocolReturn(msg, Game::ItemForFriendliness_CS::UNKNOWN, CMSGItemForFriendliness_CS);
		}

		subItemBag(msg.itemid());
		sendPlayerInfo();
		return ProtocolReturn(msg, Game::ItemForFriendliness_CS::SUCCESS, CMSGItemForFriendliness_CS);
	}

	printf("互动道具交互，物品信息: id %d, type %d, subtype %d\n", msg.itemid(), type, subtype);
	return ProtocolReturn(msg, Game::ItemForFriendliness_CS::UNKNOWN, CMSGItemForFriendliness_CS);
}