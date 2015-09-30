#include "Player.h"

#include "ClientCommand/message.pb.h"
#include "Game_Define.h"

#define EXP_RATE 0.8
#define OPEN_FOURTH_SLOT_QUALITY 4
#include "TableManager.h"

using namespace network::command;

void GamePlayer::buyCurrency(network::command::Shop::BuyCurrency_CS &msg)
{
	switch (msg.itemtype())
	{
	case Shop::Gold:
		return buyGold(msg);
	case Shop::Diamond:
		return buyDiamond(msg);
	case Shop::Energy:
		return buyEnergy(msg);
	default:
		break;
	}

//	MERROR("未解析的购买类型:%u", msg.itemtype());
}

void GamePlayer::buyGold(network::command::Shop::BuyCurrency_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("ExchangeGold");
	uint32 itemid = msg.buyitemid();
	uint32 gold = ptr->asInt(itemid, "gold");
	uint32 diamond = ptr->asInt(itemid, "diamond");
//	INFO("player: %s, itemid: %d, exchange gold, gold add: %d, diamond need: %d", m_player.name().c_str(), itemid, gold, diamond);
	if (gold == 0 || diamond == 0 || gold == IMPOSSIBLE_RETURN || diamond == IMPOSSIBLE_RETURN)
	{
		msg.set_result(Shop::BuyCurrency_CS::UNKNOWN);
	}
	else
	{
		if (subMoney(Shop::Diamond, diamond, SubDiamondForGold))
		{
			addMoney(Shop::Gold, gold, AddGoldUseDiamond);
			msg.set_result(Shop::BuyCurrency_CS::SUCCESS);
		}
		else
		{
			msg.set_result(Shop::BuyCurrency_CS::FAIL);
		}
	}

	ConstructMsgMacro(CMSGBuyCurrency_CS, msg);
	m_connection->sendCmdMsg(msg__, msg_size__);

	sendPlayerInfo();
}

// 购买钻石需要和支付平台交互
void GamePlayer::buyDiamond(network::command::Shop::BuyCurrency_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("BuyDiamond");
	uint32 itemid = msg.buyitemid();
	uint32 money = ptr->asInt(itemid, "money");
	uint32 diamond = ptr->asInt(itemid, "diamond");

//	INFO("player: %s, itemid: %d, buy energy, diamond add: %d, money need: %d", m_player.name().c_str(), itemid, diamond, money);

	if (money == 0 || diamond == 0 || money == IMPOSSIBLE_RETURN || diamond == IMPOSSIBLE_RETURN)
	{
		msg.set_result(Shop::BuyCurrency_CS::UNKNOWN);
	}
	else
	{
		addMoney(Shop::Diamond, diamond, AddDiamondUseMoney);
		msg.set_result(Shop::BuyCurrency_CS::SUCCESS);
	}

	ConstructMsgMacro(CMSGBuyCurrency_CS, msg);
	m_connection->sendCmdMsg(msg__, msg_size__);

	sendPlayerInfo();
}

void GamePlayer::buyEnergy(network::command::Shop::BuyCurrency_CS &msg)
{
	uint32 itemid = msg.buyitemid();
	auto ptr = TableManager::getInstance().getTable("BuyEnergy");
	uint32 energy = ptr->asInt(itemid, "energy");
	uint32 diamond = ptr->asInt(itemid, "diamond");

//	INFO("player: %s, itemid: %d,  buy energy, energy add: %d, diamond need: %d", m_player.name().c_str(), itemid, energy, diamond);

	if (energy == 0 || diamond == 0 || energy == IMPOSSIBLE_RETURN || diamond == IMPOSSIBLE_RETURN)
	{
		msg.set_result(Shop::BuyCurrency_CS::UNKNOWN);
	}
	else
	{
		if (subMoney(Shop::Diamond, diamond, SubDiamondForEnergy))
		{
			addEnergy(energy);
			msg.set_result(Shop::BuyCurrency_CS::SUCCESS);
		}
		else
		{
			msg.set_result(Shop::BuyCurrency_CS::FAIL);
		}
	}

	ConstructMsgMacro(CMSGBuyCurrency_CS, msg);
	m_connection->sendCmdMsg(msg__, msg_size__);

	sendPlayerInfo();
}



void GamePlayer::upgradeObject(network::command::Shop::UpgradeObject_CS &msg)
{
	switch (msg.obtype())
	{
	case Shop::UpgradeObject_CS::Pilot:
		return upgradePilot(msg);
	case Shop::UpgradeObject_CS::Airplane:
		return upgradeAirplane(msg);
	case Shop::UpgradeObject_CS::AirplanePart:
		return upgradePlanePart(msg);
	default:
		break;
	}
//	MERROR("未解析的部件类型:%u", msg.obtype());
}

// 读取pilot表格，获取升级要求，检查下级的项目是否存在，然后扣除金钱，返回
void GamePlayer::upgradePilot(network::command::Shop::UpgradeObject_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("PilotProperty");
	uint32 id = ptr->asInt(msg.id(), "pilot_id");
//	INFO("角色升级ID: %d", id);

	if (id == IMPOSSIBLE_RETURN || id != msg.id())
	{
//		INFO("需要升级角色在表格中不存在: %d", id);

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
	}

	Pilot *pilot = getPilot(id);

	if (pilot == NULL)
	{
//		INFO("玩家之前不拥有该角色: %d", id);

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNEXIST, CMSGUpgradeObject_CS);
	}

	uint32 can_levelup = ptr->asInt(id, "can_levelup");

	if (msg.uptype() == Shop::UpgradeObject_CS::Level && can_levelup == 1)
	{
		uint32 next_id = ptr->asInt(id + 1, "pilot_id");
		if (next_id == IMPOSSIBLE_RETURN || next_id != id + 1)
		{
//			INFO("升级角色在表格中不存在: %d", next_id);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}

		uint32 gold = ptr->asInt(id, "gold");
		uint32 diamond = ptr->asInt(id, "diamond");

		gold = gold == (uint32)-1 ? 0 : gold;
		diamond = diamond == (uint32)-1 ? 0 : diamond;

		bool ok = checkMoney(Shop::Gold, gold);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NOGOLD, CMSGUpgradeObject_CS);
		}

		ok = checkMoney(Shop::Diamond, diamond);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NODIAMOND, CMSGUpgradeObject_CS);
		}

		subMoney(Shop::Gold, gold, SubGoldForPilotUpgrade);
		subMoney(Shop::Diamond, diamond, SubDiamondForPilotUpgrade);

		pilot->set_id(next_id);
		m_player.set_currentpilot(next_id);

		sendPlayerInfo();

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::SUCCESS, CMSGUpgradeObject_CS);
	}
	else if (msg.uptype() == Shop::UpgradeObject_CS::Quality)
	{
//		INFO("角色不存在升阶: %d", id);

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
	}
	else if (msg.uptype() == Shop::UpgradeObject_CS::OneClick && can_levelup == 1)
	{
		uint32 next_id = id;
		uint32 check_id = next_id;
		uint32 gold = 0;
		uint32 add_gold = 0;
		uint32 diamond = 0;
		uint32 add_diamond = 0;
		bool ok = false;
		do
		{
			next_id = ptr->asInt(check_id + 1, "pilot_id");
			if (next_id == IMPOSSIBLE_RETURN || next_id != check_id + 1)
			{
//				INFO("升级角色在表格中不存在: %d", next_id);
				break;
			}

			add_gold = ptr->asInt(check_id, "gold");
			add_diamond = ptr->asInt(check_id, "diamond");

			if (add_gold != IMPOSSIBLE_RETURN)
				gold += add_gold;
			if (add_diamond != IMPOSSIBLE_RETURN)
				diamond += add_diamond;

			ok = checkMoney(Shop::Gold, gold);
			ok = ok && checkMoney(Shop::Diamond, diamond);
			if (!ok)
			{
				gold -= ptr->asInt(check_id, "gold");
				diamond -= ptr->asInt(check_id, "diamond");
				break;
			}

			can_levelup = ptr->asInt(next_id, "can_levelup");
			check_id = next_id;
		} while (can_levelup == 1);

		if (check_id == id)
		{
//			INFO("升级失败，%d", id);
			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}

		subMoney(Shop::Gold, gold, SubGoldForPilotUpgrade);
		subMoney(Shop::Diamond, diamond, SubDiamondForPilotUpgrade);

		pilot->set_id(check_id);
		m_player.set_currentpilot(check_id);

		sendPlayerInfo();

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::SUCCESS, CMSGUpgradeObject_CS);
	}
	else if (msg.uptype() == Shop::UpgradeObject_CS::Skill)
	{
		uint32 skill_id = pilot->skill_property_id();
		if (skill_id == 0)
		{
			skill_id = ptr->asInt(id, "skill_property_id");
		}

		if (skill_id == IMPOSSIBLE_RETURN)
		{
//			INFO("需要升级技能在角色配置中不存在: %d", id);
			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}

		auto psptr = TableManager::getInstance().getTable("PilotSkillProperty");
		uint32 gold = psptr->asInt(skill_id, "costgold");
		uint32 diamond = psptr->asInt(skill_id, "costdiamond");

		if (gold == IMPOSSIBLE_RETURN && diamond == IMPOSSIBLE_RETURN)
		{
//			INFO("需要升级技能在表格中不存在: %d", id);
			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}

		gold = gold == (uint32)-1 ? 0 : gold;
		diamond = diamond == (uint32)-1 ? 0 : diamond;

		bool ok = checkMoney(Shop::Gold, gold);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NOGOLD, CMSGUpgradeObject_CS);
		}

		ok = checkMoney(Shop::Diamond, diamond);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NODIAMOND, CMSGUpgradeObject_CS);
		}

		subMoney(Shop::Gold, gold, SubGoldForSkill);
		subMoney(Shop::Diamond, diamond, SubDiamondForSkill);

		pilot->set_skill_property_id(skill_id + 1);
		sendPlayerInfo();

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::SUCCESS, CMSGUpgradeObject_CS);
	}

//	INFO("升级?: can_levelup, %d; uptype, %d", can_levelup, msg.uptype());

	return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
}

void GamePlayer::upgradeAirplane(network::command::Shop::UpgradeObject_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("PlaneProperty");

	uint32 id = ptr->asInt(msg.id(), "plane_id");
//	INFO("飞行器升级升阶ID: %d", id);
	if (id == IMPOSSIBLE_RETURN || id != msg.id())
	{
//		INFO("需要升级飞机在表格中不存在: %d", id);

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
	}

	Airplane *airplane = getPlane(id);

	if (airplane == nullptr)
	{
//		INFO("玩家之前不拥有该飞机: %d", id);

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNEXIST, CMSGUpgradeObject_CS);
	}

	uint32 can_upgrade = ptr->asInt(id, "can_upgrade");

	if (msg.uptype() == Shop::UpgradeObject_CS::Level && can_upgrade == 0)
	{
		uint32 next_id = ptr->asInt(id + 1, "plane_id");
		if (next_id == IMPOSSIBLE_RETURN || next_id != id + 1)
		{
//			INFO("升级飞机在表格中不存在: %d", next_id);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}

		uint32 gold = ptr->asInt(id, "levelup_money");
		uint32 diamond = ptr->asInt(id, "levelup_diamond");

		gold = gold == IMPOSSIBLE_RETURN ? 0 : gold;
		diamond = diamond == IMPOSSIBLE_RETURN ? 0 : diamond;

		bool ok = checkMoney(Shop::Gold, gold);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NOGOLD, CMSGUpgradeObject_CS);
		}

		ok = checkMoney(Shop::Diamond, diamond);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NODIAMOND, CMSGUpgradeObject_CS);
		}

		subMoney(Shop::Gold, gold, SubGoldForAirplaneUpgrade);
		subMoney(Shop::Diamond, diamond, SubDiamondForAirplaneUpgrade);

		airplane->set_id(next_id);
		m_player.set_currentairplane(next_id);

		sendPlayerInfo();

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::SUCCESS, CMSGUpgradeObject_CS);
	}
	else if (msg.uptype() == Shop::UpgradeObject_CS::Quality && can_upgrade == 1)
	{
		uint32 upgrade_id = ptr->asInt(id, "upgrade_plane_id");
		if (upgrade_id == IMPOSSIBLE_RETURN)
		{
//			INFO("进阶飞机在表格中不存在: %d", upgrade_id);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}
		// upgrade_id可能为-1，也可能为错误的值，检查下
		uint32 next_id = ptr->asInt(upgrade_id, "plane_id");
		if (next_id != upgrade_id)
		{
//			INFO("进阶飞机在表格中不存在: %d", next_id);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}

		uint32 gold = ptr->asInt(id, "upgrade_money");
		uint32 diamond = ptr->asInt(id, "upgrade_diamond");

		gold = gold == (uint32)-1 ? 0 : gold;
		diamond = diamond == (uint32)-1 ? 0 : diamond;

		bool ok = checkMoney(Shop::Gold, gold);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NOGOLD, CMSGUpgradeObject_CS);
		}

		ok = checkMoney(Shop::Diamond, diamond);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NODIAMOND, CMSGUpgradeObject_CS);
		}

		subMoney(Shop::Gold, gold, SubGoldForAirplaneUpgrade);
		subMoney(Shop::Diamond, diamond, SubDiamondForAirplaneUpgrade);

		airplane->set_id(next_id);
		m_player.set_currentairplane(next_id);

		// S级开启飞机第4个槽
		if (airplane->slotfour().isopen() == false)
		{
			uint32 quality = ptr->asInt(next_id, "quality");
			if (quality == OPEN_FOURTH_SLOT_QUALITY)
				airplane->mutable_slotfour()->set_isopen(true);
		}

		sendPlayerInfo();

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::SUCCESS, CMSGUpgradeObject_CS);
	}

	if (msg.uptype() == Shop::UpgradeObject_CS::Slot)
	{
		if (airplane->slotfive().isopen() == true)
		{
//			INFO("购买槽已经打开");
			return ProtocolReturn(msg, Shop::UpgradeObject_CS::SLOTISOPEN, CMSGUpgradeObject_CS);
		}

		uint32 diamond = ptr->asInt(id, "position_demon");
		if (diamond == IMPOSSIBLE_RETURN)
		{
//			INFO("购买槽钻石数据错误，%d, %d", id, diamond);
			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}

		bool ok = checkMoney(Shop::Diamond, diamond);
		if (!ok)
		{
//			INFO("金钱不足:diamond, %d", diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NODIAMOND, CMSGUpgradeObject_CS);
		}

		subMoney(Shop::Diamond, diamond, SubDiamondForAirplaneUpgrade);

		airplane->mutable_slotfive()->set_isopen(true);
		m_player.set_currentairplane(id);

		sendPlayerInfo();

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::SUCCESS, CMSGUpgradeObject_CS);
	}

//	INFO("升级还是进阶: can_upgrade, %d; uptype, %d", can_upgrade, msg.uptype());
	return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
}

void GamePlayer::upgradePlanePart(network::command::Shop::UpgradeObject_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("PlanePartsProperty");
	uint32 id = ptr->asInt(msg.id(), "engineid");
	if (id != msg.id() || id == IMPOSSIBLE_RETURN)
	{
//		INFO("该飞机部件编号在飞机部件表中不存在, %d, %d", msg.id(), id);
		return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
	}

	PlanePart *part = NULL;
	Bag *bag = m_player.mutable_bag();
	if (msg.airplaneid() != -1)
	{
		Airplane *airplane = getPlane(msg.airplaneid());
		if (airplane == NULL)
		{
//			INFO("玩家之前不拥有该飞机: %d", msg.airplaneid());

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNEXIST, CMSGUpgradeObject_CS);
		}

		m_player.set_currentairplane((uint32)msg.airplaneid());

		switch (msg.slot())
		{
		case Shop::SLOTONE:
		{
			if (airplane->slotone().isopen() == true)
				part = airplane->mutable_slotone()->mutable_part();
			break;
		}
		case Shop::SLOTTWO:
		{
			if (airplane->slottwo().isopen() == true)
				part = airplane->mutable_slottwo()->mutable_part();
			break;
		}
		case Shop::SLOTTHREE:
		{
			if (airplane->slotthree().isopen() == true)
				part = airplane->mutable_slotthree()->mutable_part();
			break;
		}
		case Shop::SLOTFOUR:
		{
			if (airplane->slotfour().isopen() == true)
				part = airplane->mutable_slotfour()->mutable_part();
			break;
		}
		case Shop::SLOTFIVE:
		{
			if (airplane->slotfive().isopen() == true)
				part = airplane->mutable_slotfive()->mutable_part();
			break;
		}
		}
	}
	else
	{
		::google::protobuf::RepeatedPtrField< ::network::command::PlanePart >* partlist = bag->mutable_partlist();
		::google::protobuf::RepeatedPtrField< PlanePart >::iterator it_part = partlist->begin();
		for (; it_part != partlist->end(); it_part++)
		{
			if (it_part->id() == id && it_part->thisid() == msg.thisid())
			{
				part = &(*it_part);
				break;
			}
		}
	}

	if (part == nullptr)
	{
//		INFO("该飞机部件在用户身上中不存在, %d, %d", msg.id(), msg.thisid());
		return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNEXIST, CMSGUpgradeObject_CS);
	}

	uint32 can_rankup = ptr->asInt(id, "canrankup");

	if (msg.uptype() == Shop::UpgradeObject_CS::Level)
	{
		uint32 exp = ptr->asInt(id, "exp");
		if (part->exp() == exp && can_rankup != 0)
		{
//			INFO("该部件已经升级到顶级, %d", id);
			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}

		uint32 add_exp = 0;

		::google::protobuf::RepeatedPtrField< ::network::command::Shop::upgradeItem >* upitemlist = msg.mutable_itemlist();
		::google::protobuf::RepeatedPtrField< Shop::upgradeItem >::iterator it_item = upitemlist->begin();

		int partlist_size = bag->partlist_size();
		uint32 tmp_exp = 0;
		for (; it_item != upitemlist->end(); ++it_item)
		{
			if (it_item->thisid() != 0)
			{
				for (int i = 0; i != partlist_size; ++i)
				{
					if (bag->partlist(i).thisid() == it_item->thisid())
					{
						if (it_item->thisid() == part->thisid())
						{
//							INFO("不能吞噬自身, %d", part->thisid());
							return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
						}
						tmp_exp = ptr->asInt(it_item->id(), "exp");
						if (tmp_exp != IMPOSSIBLE_RETURN)
							add_exp += static_cast<uint32>(tmp_exp * EXP_RATE);
						break;
					}
				}
			}
			else
			{
				uint32 exp = TableManager::getInstance().getTable("PlayerItem")->asInt(it_item->id(), "exp");
				if (exp != IMPOSSIBLE_RETURN)
				{
					add_exp += exp;
				}
			}
		}

		uint32 next_id = id;
		uint32 max_exp = ptr->asInt(next_id, "exp");
		uint32 total_exp = add_exp + part->exp();
		while (max_exp <= total_exp)
		{
			next_id++;
			can_rankup = ptr->asInt(next_id, "canrankup");
			max_exp = ptr->asInt(next_id, "exp");
			if (can_rankup != 0)
				break;
		}

		uint32 total_gold = 0;
		uint32 total_diamond = 0;
		uint32 gold = 0;
		uint32 diamond = 0;
		for (uint32 i = id; i < next_id; ++i)
		{
			gold = ptr->asInt(i, "levelupgold");
			diamond = ptr->asInt(i, "levelupdiamond");
			if (gold != IMPOSSIBLE_RETURN)
				total_gold += gold;
			if (diamond != IMPOSSIBLE_RETURN)
				total_diamond += diamond;
		}

		gold = gold == IMPOSSIBLE_RETURN ? 0 : gold;
		diamond = diamond == IMPOSSIBLE_RETURN ? 0 : diamond;

		bool ok = checkMoney(Shop::Gold, total_gold);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", total_gold, total_diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NOGOLD, CMSGUpgradeObject_CS);
		}

		ok = checkMoney(Shop::Diamond, total_diamond);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", total_gold, total_diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NODIAMOND, CMSGUpgradeObject_CS);
		}

		for (it_item = upitemlist->begin(); it_item != upitemlist->end(); ++it_item)
		{
			if (it_item->thisid() != 0)
			{
				subPlanePartBag(it_item->thisid());
			}
			else
			{
				subNormalItemBag(it_item->id());
			}
		}

		subMoney(Shop::Gold, total_gold, SubGoldForPartsUpgrade);
		subMoney(Shop::Diamond, total_diamond, SubDiamondForPartsUpgrade);

		part->set_exp(total_exp > max_exp ? max_exp : total_exp);
		part->set_id(next_id);

//		INFO("升级成功：%d", msg.id());
		sendPlayerInfo();

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::SUCCESS, CMSGUpgradeObject_CS);
	}
	else if (msg.uptype() == Shop::UpgradeObject_CS::Quality && can_rankup == 1)
	{
		uint32 upgrade_id = ptr->asInt(id, "rankupid");
		if (upgrade_id == IMPOSSIBLE_RETURN)
		{
//			INFO("进阶飞机部件在表格中不存在: %d", upgrade_id);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}
		// upgrade_id可能为-1，也可能为错误的值，检查下
		uint32 next_id = ptr->asInt(upgrade_id, "engineid");
		if (next_id != upgrade_id)
		{
//			INFO("进阶飞机部件在表格中不存在: %d", next_id);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
		}

		uint32 gold = ptr->asInt(id, "rankupgold");
		uint32 diamond = ptr->asInt(id, "rankupdiamond");
		uint32 prop_id = ptr->asInt(id, "rankupprop");
		uint32 prop_num = ptr->asInt(id, "rankupnumber");

		gold = gold == IMPOSSIBLE_RETURN ? 0 : gold;
		diamond = diamond == IMPOSSIBLE_RETURN ? 0 : diamond;

		bool ok = checkMoney(Shop::Gold, gold);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NOGOLD, CMSGUpgradeObject_CS);
		}

		ok = checkMoney(Shop::Diamond, diamond);
		if (!ok)
		{
//			INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

			return ProtocolReturn(msg, Shop::UpgradeObject_CS::NODIAMOND, CMSGUpgradeObject_CS);
		}

		if (prop_id != (uint32)-1 && prop_num != (uint32)-1 && prop_id != IMPOSSIBLE_RETURN && prop_num != IMPOSSIBLE_RETURN)
		{
//			INFO("扣除道具:%d, 数量:%d", prop_id, prop_num);
			if (!subItemBag(prop_id, prop_num))
			{
//				INFO("升阶物品在包裹中数量不足, id: %d, num: %d", prop_id, prop_num);
				return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
			}
		}

		subMoney(Shop::Gold, gold, SubGoldForPartsUpgrade);
		subMoney(Shop::Diamond, diamond, SubDiamondForPartsUpgrade);

		part->set_id(next_id);

		sendPlayerInfo();

		return ProtocolReturn(msg, Shop::UpgradeObject_CS::SUCCESS, CMSGUpgradeObject_CS);
	}

//	INFO("升级还是进阶: can_rankup, %d; uptype, %d", can_rankup, msg.uptype());
	return ProtocolReturn(msg, Shop::UpgradeObject_CS::UNKNOWN, CMSGUpgradeObject_CS);
}

void GamePlayer::buyObject(network::command::Shop::BuyObject_CS &msg)
{
	if (msg.type() == Shop::BuyObject_CS::Airplane)
	{
		buyAirplane(msg);
		return;
	}
	if (msg.type() == Shop::BuyObject_CS::Pilot)
	{
		buyPilot(msg);
		return;
	}
	if (msg.type() == Shop::BuyObject_CS::BagSpace)
	{
		buyBagSpace(msg);
		return;
	}
	if (msg.type() == Shop::BuyObject_CS::PetScrawl)
	{
//		buyPetScrawl(msg);
		return;
	}

	if (bagHasFull())
	{
//		INFO("背包空间已满");
		return ProtocolReturn(msg, Shop::BuyObject_CS::NOSPACE, CMSGBuyObject_CS);
	}
	else
	{
		if (msg.type() == Shop::BuyObject_CS::PlanePart)
		{
			buyPlanePart(msg);
			return;
		}
		if (msg.type() == Shop::BuyObject_CS::Item)
		{
			buyItem(msg);
			return;
		}
		if (msg.type() == Shop::BuyObject_CS::PileItem)
		{
			buyPileItem(msg);
			return;
		}
		if (msg.type() == Shop::BuyObject_CS::PetEgg)
		{
//			buyPetEgg(msg);
			return;
		}
	}

//	DEBUG("暂时还不支持这些购买类型, %d", msg.type());
}

void GamePlayer::buyAirplane(network::command::Shop::BuyObject_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("BuyPlane");
	uint32 id = ptr->asInt(msg.id(), "airplane_id");
	if (id != msg.id() || id == IMPOSSIBLE_RETURN)
	{
//		INFO("该飞机编号在飞机购买表中不存在, %d", msg.id());

		return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
	}

	uint32 tmp_id = TableManager::getInstance().getTable("PlaneProperty")->asInt(id, "plane_id");
	if (tmp_id == IMPOSSIBLE_RETURN || tmp_id != id)
	{
//		INFO("购买的飞机编号在飞机属性表中不存在, %d", id);

		return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
	}

	if (getPlane(id))
	{
//		INFO("购买的飞机已经存在, %d", id);
		return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
	}

	uint32 level = ptr->asInt(id, "level");
	uint32 gold = ptr->asInt(id, "gold");
	uint32 diamond = ptr->asInt(id, "diamond");

	gold = gold == (uint32)-1 ? 0 : gold;
	diamond = diamond == (uint32)-1 ? 0 : diamond;

	if (m_player.level() < level)
	{
//		INFO("等级不足, 当前等级%d, 需要等级%d", m_player.level(), level);

		return ProtocolReturn(msg, Shop::BuyObject_CS::NODIAMOND, CMSGBuyObject_CS);
	}

	bool ok = checkMoney(Shop::Gold, gold);
	ok = ok && checkMoney(Shop::Diamond, diamond);

	if (!ok)
	{
//		INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);

		return ProtocolReturn(msg, Shop::BuyObject_CS::NOGOLD, CMSGBuyObject_CS);
	}

	subMoney(Shop::Gold, gold, SubGoldForAirplaneBuy);
	subMoney(Shop::Diamond, diamond, SubDiamondForAirplaneBuy);

	addAirPlane(id, true);

	sendPlayerInfo();

	return ProtocolReturn(msg, Shop::BuyObject_CS::SUCCESS, CMSGBuyObject_CS);
}

void GamePlayer::buyPilot(network::command::Shop::BuyObject_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("BuyPilot");
	uint32 pilot_id = ptr->asInt(msg.id(), "pilot_id");
	if (pilot_id != msg.id() || pilot_id == IMPOSSIBLE_RETURN)
	{
//		INFO("该角色编号在角色购买表中不存在, %d", msg.id());

		return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
	}

	auto ppptr = TableManager::getInstance().getTable("PilotProperty");
	uint32 tmp_id = ppptr->asInt(pilot_id, "pilot_id");
	if (pilot_id == IMPOSSIBLE_RETURN || tmp_id == IMPOSSIBLE_RETURN || tmp_id != pilot_id)
	{
//		INFO("购买的角色编号在角色属性表中不存在, %d", pilot_id);

		return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
	}

	uint32 level = ptr->asInt(pilot_id, "level");
	uint32 gold = ptr->asInt(pilot_id, "gold");
	uint32 diamond = ptr->asInt(pilot_id, "diamond");
	uint32 pre_pilot_id = ptr->asInt(pilot_id, "pre_pilot_id");
	uint32 pre_pilot_level = ptr->asInt(pilot_id, "pre_pilot_level");

	if (m_player.level() < level)
	{
//		INFO("等级不足, 当前等级%d, 需要等级%d", m_player.level(), level);

		return ProtocolReturn(msg, Shop::BuyObject_CS::PLAYER_LEVEL, CMSGBuyObject_CS);
	}

	if (pre_pilot_id != (uint32)-1)
	{
		tmp_id = ppptr->asInt(pre_pilot_id, "pilot_id");
		if (tmp_id == IMPOSSIBLE_RETURN || tmp_id != pre_pilot_id)
		{
//			INFO("前置角色在角色属性表中不存在, %d", pre_pilot_id);

			return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
		}

		Pilot *pilot = getPilot(pre_pilot_id);

		if (pilot == nullptr)
		{
//			INFO("玩家没有前置角色, %d", pre_pilot_id);

			return ProtocolReturn(msg, Shop::BuyObject_CS::PRE_PILOT_NOEXIST, CMSGBuyObject_CS);
		}

		if (pre_pilot_level != IMPOSSIBLE_RETURN)
		{
			uint32 tmp_level = ppptr->asInt(pre_pilot_id, "pilot_id");
			if (tmp_level < pre_pilot_level)
			{
//				INFO("前置角色等级条件没有达到, 当前等级:%d, 需求等级:%d", tmp_level, pre_pilot_level);

				return ProtocolReturn(msg, Shop::BuyObject_CS::PRE_PILOT_LEVEL, CMSGBuyObject_CS);
			}
		}
	}

	gold = gold == IMPOSSIBLE_RETURN ? 0 : gold;
	diamond = diamond == IMPOSSIBLE_RETURN ? 0 : diamond;

	bool ok = checkMoney(Shop::Gold, gold);
	if (!ok)
	{
//		INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);
		return ProtocolReturn(msg, Shop::BuyObject_CS::NOGOLD, CMSGBuyObject_CS);
	}

	ok = checkMoney(Shop::Diamond, diamond);
	if (!ok)
	{
//		INFO("金钱不足: gold, %d; diamond, %d", gold, diamond);
		return ProtocolReturn(msg, Shop::BuyObject_CS::NODIAMOND, CMSGBuyObject_CS);
	}

	subMoney(Shop::Gold, gold, SubGoldForPilotBuy);
	subMoney(Shop::Diamond, diamond, SubDiamondForPilotBuy);

	Pilot *pilot = m_player.add_pilotlist();
	pilot->set_id(pilot_id);
	pilot->set_skill_property_id(ppptr->asInt(pilot_id, "skill_property_id"));
	m_player.set_currentpilot(pilot_id);

	sendPlayerInfo();

	return ProtocolReturn(msg, Shop::BuyObject_CS::SUCCESS, CMSGBuyObject_CS);
}

void GamePlayer::buyPlanePart(network::command::Shop::BuyObject_CS &msg)
{
	bool ok = addItemToBag(msg.id());
	if (ok)
	{
		sendPlayerInfo();
		return ProtocolReturn(msg, Shop::BuyObject_CS::SUCCESS, CMSGBuyObject_CS);
	}

	return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
}

void GamePlayer::buyItem(network::command::Shop::BuyObject_CS &msg)
{
	bool ok = addItemToBag(msg.id());
	if (ok)
	{
		sendPlayerInfo();
		return ProtocolReturn(msg, Shop::BuyObject_CS::SUCCESS, CMSGBuyObject_CS);
	}

	return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
}

void GamePlayer::buyPileItem(network::command::Shop::BuyObject_CS &msg)
{
	bool ok = addItemToBag(msg.id());
	if (ok)
	{
		sendPlayerInfo();
		return ProtocolReturn(msg, Shop::BuyObject_CS::SUCCESS, CMSGBuyObject_CS);
	}

	return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
}

void GamePlayer::buyBagSpace(network::command::Shop::BuyObject_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("BuyBagSpace");
	uint32 id = ptr->asInt(msg.id(), "id");
	if (id == IMPOSSIBLE_RETURN || id != msg.id())
	{
//		INFO("购买空间行不存在: %d", id);

		return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
	}

	uint32 diamond = ptr->asInt(id, "diamond");
	uint32 space = ptr->asInt(id, "space");
//	INFO("player: %s, id: %d, space add: %d, diamond need: %d", m_player.name().c_str(), id, space, diamond);

	diamond = diamond == (uint32)-1 ? 0 : diamond;
	if (diamond == 0 || space == 0)
	{
		return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
	}

	bool ok = checkMoney(Shop::Diamond, diamond);
	if (!ok)
	{
//		INFO("金钱不足: diamond, %d", diamond);
		return ProtocolReturn(msg, Shop::BuyObject_CS::NODIAMOND, CMSGBuyObject_CS);
	}

	uint32 bag_max_size = TableManager::getInstance().getTable("PlayerInit")->asInt(0, "bag_max_size");
	if (m_player.bag().size() == bag_max_size)
	{
//		INFO("已经达到了最大的包裹空间量, %d", bag_max_size);
		return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
	}

	subMoney(Shop::Diamond, diamond, SubDiamondForPilotBuy);

	ok = addBagSpace(space);

	if (ok)
	{
		sendPlayerInfo();
		return ProtocolReturn(msg, Shop::BuyObject_CS::SUCCESS, CMSGBuyObject_CS);
	}

	return ProtocolReturn(msg, Shop::BuyObject_CS::UNKNOWN, CMSGBuyObject_CS);
}



void GamePlayer::installPlanePart(network::command::Shop::InstallPlanePart_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("PilotProperty");
	uint32 id = ptr->asInt(msg.planeid(), "plane_id");
//	INFO("安装部件的飞行器ID: %d", id);
	if (id == IMPOSSIBLE_RETURN || id != msg.planeid())
	{
//		INFO("需要安装部件的飞机在表格中不存在: %d", id);

		return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
	}

	Airplane *airplane = getPlane(id);
	if (airplane == NULL)
	{
//		INFO("安装部件失败，玩家不拥有该飞机: %d", id);

		return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
	}

	if (msg.optype() == Shop::InstallPlanePart_CS::INSTALL)
	{
		Bag *bag = m_player.mutable_bag();
		PlanePart *part = NULL;
		::google::protobuf::RepeatedPtrField< ::network::command::PlanePart >* partlist = bag->mutable_partlist();
		::google::protobuf::RepeatedPtrField< PlanePart >::iterator it_part = partlist->begin();
		for (; it_part != partlist->end(); it_part++)
		{
			if (it_part->thisid() == msg.thisid())
			{
				part = &(*it_part);
				break;
			}
		}

		if (part == nullptr)
		{
			Airplane *ex_plane = getPlane(msg.explaneid());
			if (ex_plane)
			{
				switch (msg.exslot())
				{
				case Shop::SLOTONE:
					part = ex_plane->mutable_slotone()->mutable_part();
					break;
				case Shop::SLOTTWO:
					part = ex_plane->mutable_slottwo()->mutable_part();
					break;
				case Shop::SLOTTHREE:
					part = ex_plane->mutable_slotthree()->mutable_part();
					break;
				case Shop::SLOTFOUR:
					part = ex_plane->mutable_slotfour()->mutable_part();
					break;
				case Shop::SLOTFIVE:
					part = ex_plane->mutable_slotfive()->mutable_part();
					break;
				}
			}
		}

		if (part == nullptr)
		{
//			INFO("没有发送正确的thisid过来，或者交换的飞机id和槽id不对，%d，%d, %d", msg.thisid(), msg.explaneid(), msg.exslot());
			return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
		}

		switch (msg.slot())
		{
		case Shop::SLOTONE:
		{
			if (airplane->slotone().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());
				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			planePartInstall(msg, airplane->mutable_slotone(), part);

			break;
		}
		case Shop::SLOTTWO:
		{
			if (airplane->slottwo().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());
				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			planePartInstall(msg, airplane->mutable_slottwo(), part);

			break;
		}
		case Shop::SLOTTHREE:
		{
			if (airplane->slotthree().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());
				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			planePartInstall(msg, airplane->mutable_slotthree(), part);

			break;
		}
		case Shop::SLOTFOUR:
		{
			if (airplane->slotfour().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());

				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			planePartInstall(msg, airplane->mutable_slotfour(), part);

			break;
		}
		case Shop::SLOTFIVE:
		{
			if (airplane->slotfive().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());
				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			planePartInstall(msg, airplane->mutable_slotfive(), part);

			break;
		}
		default:
//			INFO("未知槽，%d", msg.slot());
			return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
		}

		sendPlayerInfo();
		return ProtocolReturn(msg, Shop::InstallPlanePart_CS::SUCCESS, CMSGInstallPlanePart_CS);
	}
	else if (msg.optype() == Shop::InstallPlanePart_CS::UNINSTALL)
	{
		switch (msg.slot())
		{
		case Shop::SLOTONE:
		{
			if (airplane->slotone().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());
				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			Bag *bag = m_player.mutable_bag();
			PlanePart *part = bag->add_partlist();
			part->CopyFrom(airplane->slotone().part());
			airplane->mutable_slotone()->clear_part();

			break;
		}
		case Shop::SLOTTWO:
		{
			if (airplane->slottwo().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());
				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			Bag *bag = m_player.mutable_bag();
			PlanePart *part = bag->add_partlist();
			part->CopyFrom(airplane->slottwo().part());
			airplane->mutable_slottwo()->clear_part();

			break;
		}
		case Shop::SLOTTHREE:
		{
			if (airplane->slotthree().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());
				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			Bag *bag = m_player.mutable_bag();
			PlanePart *part = bag->add_partlist();
			part->CopyFrom(airplane->slotthree().part());
			airplane->mutable_slotthree()->clear_part();

			break;
		}
		case Shop::SLOTFOUR:
		{
			if (airplane->slotfour().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());
				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			Bag *bag = m_player.mutable_bag();
			PlanePart *part = bag->add_partlist();
			part->CopyFrom(airplane->slotfour().part());
			airplane->mutable_slotfour()->clear_part();

			break;
		}
		case Shop::SLOTFIVE:
		{
			if (airplane->slotfive().isopen() == false)
			{
//				INFO("槽未开启，%d", msg.slot());
				return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
			}

			Bag *bag = m_player.mutable_bag();
			PlanePart *part = bag->add_partlist();
			part->CopyFrom(airplane->slotfive().part());
			airplane->mutable_slotfive()->clear_part();

			break;
		}
		default:
//			INFO("未知槽，%d", msg.slot());
			return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
		}

		sendPlayerInfo();
		return ProtocolReturn(msg, Shop::InstallPlanePart_CS::SUCCESS, CMSGInstallPlanePart_CS);
	}

	return ProtocolReturn(msg, Shop::InstallPlanePart_CS::UNKNOWN, CMSGInstallPlanePart_CS);
}

void GamePlayer::planePartInstall(Shop::InstallPlanePart_CS &msg, PlaneSlot *slot, PlanePart *part)
{
	if (slot->has_part())
	{
		part->Swap(slot->mutable_part());
	}
	else
	{
		PlanePart *install_part = new PlanePart(*part);
		slot->set_allocated_part(install_part);

		Airplane *ex_plane = getPlane(msg.explaneid());
		if (ex_plane)
		{
			switch (msg.exslot())
			{
			case Shop::SLOTONE:
				ex_plane->mutable_slotone()->clear_part();
				break;
			case Shop::SLOTTWO:
				ex_plane->mutable_slottwo()->clear_part();
				break;
			case Shop::SLOTTHREE:
				ex_plane->mutable_slotthree()->clear_part();
				break;
			case Shop::SLOTFOUR:
				ex_plane->mutable_slotfour()->clear_part();
				break;
			case Shop::SLOTFIVE:
				ex_plane->mutable_slotfive()->clear_part();
				break;
			}
		}
		else
		{
			subPlanePartBag(msg.thisid());
		}
	}
}



void GamePlayer::sellObject(network::command::Shop::SellObject_CS &msg)
{
	if (msg.type() == Shop::SellObject_CS::PlanePart)
	{
		sellPlanePart(msg);
		return;
	}
	if (msg.type() == Shop::SellObject_CS::Item)
	{
		sellItem(msg);
		return;
	}

//	DEBUG("暂时还不支持这些购买类型, %d", msg.type());
}

void GamePlayer::sellItem(network::command::Shop::SellObject_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("PlayerItem");
	uint32 id = ptr->asInt(msg.id(), "itemid");
	if (id != msg.id() || id == IMPOSSIBLE_RETURN)
	{
//		INFO("%s: 物品编号不存在, %d", m_player.name().c_str(), id);
		return ProtocolReturn(msg, Shop::SellObject_CS::UNKNOWN, CMSGSellObject_CS);
	}

	uint32 can_sold = ptr->asInt(msg.id(), "can_sold");
	uint32 sell_gold = ptr->asInt(msg.id(), "sell_gold");

	if (can_sold == 0 || sell_gold == (uint32)-1 || sell_gold == IMPOSSIBLE_RETURN)
	{
//		INFO("%s, 物品不可销售，或者表格有错误, %d, %d, %d", m_player.name().c_str(), id, can_sold, sell_gold);
		return ProtocolReturn(msg, Shop::SellObject_CS::UNKNOWN, CMSGSellObject_CS);
	}

	uint32 pile_num = ptr->asInt(msg.id(), "pile_num");
	bool ok = false;

	if (pile_num == 1)
		ok = subNormalItemBag(msg.id(), msg.num());
	else
		ok = subPileItemBag(msg.id(), msg.num());

	if (ok)
	{
		addMoney(Shop::Gold, sell_gold * msg.num(), AddGoldSellObject);
		sendPlayerInfo();
		return ProtocolReturn(msg, Shop::SellObject_CS::SUCCESS, CMSGSellObject_CS);
	}

	return ProtocolReturn(msg, Shop::SellObject_CS::UNKNOWN, CMSGSellObject_CS);
}

void GamePlayer::sellPlanePart(network::command::Shop::SellObject_CS &msg)
{
	auto ptr = TableManager::getInstance().getTable("PlanePartsProperty");
	uint32 id = ptr->asInt(msg.id(), "engineid");
	if (id != msg.id() || id == IMPOSSIBLE_RETURN)
	{
//		INFO("该飞机部件编号在飞机部件表中不存在, %d", msg.id());
		return ProtocolReturn(msg, Shop::SellObject_CS::UNKNOWN, CMSGSellObject_CS);
	}

	PlanePart *part = NULL;
	Bag *bag = m_player.mutable_bag();

	::google::protobuf::RepeatedPtrField< ::network::command::PlanePart >* partlist = bag->mutable_partlist();
	::google::protobuf::RepeatedPtrField< PlanePart >::iterator it_part = partlist->begin();
	for (; it_part != partlist->end(); it_part++)
	{
		if (it_part->id() == id && it_part->thisid() == msg.thisid())
		{
			part = &(*it_part);
			break;
		}
	}

	if (part == NULL)
	{
//		INFO("该飞机部件在用户身上中不存在, %d, %d", msg.id(), msg.thisid());
		return ProtocolReturn(msg, Shop::SellObject_CS::UNEXIST, CMSGSellObject_CS);
	}

	uint32 sellgold = ptr->asInt(id, "sellgold");

	if (sellgold == IMPOSSIBLE_RETURN || sellgold == (uint32)-1)
	{
//		INFO("出售金币数量不对, %d, %d", id, sellgold);
		return ProtocolReturn(msg, Shop::SellObject_CS::UNKNOWN, CMSGSellObject_CS);
	}

	subPlanePartBag(msg.thisid());
	addMoney(Shop::Gold, sellgold, AddGoldSellObject);
	sendPlayerInfo();

	return ProtocolReturn(msg, Shop::SellObject_CS::SUCCESS, CMSGSellObject_CS);
}









bool GamePlayer::checkMoney(network::command::Shop::CurrencyType type, const uint32 num) const
{
//	CheckCondition(network::command::Shop::Gold == type || network::command::Shop::Diamond == type, false);
	if (network::command::Shop::Gold == type)
		return m_player.gold() >= num;
	else
		return m_player.diamond() >= num;
}

void GamePlayer::addMoney(network::command::Shop::CurrencyType type, uint32 num, MoneyAction action)
{
//	CheckConditionVoid(network::command::Shop::Gold == type || network::command::Shop::Diamond == type || network::command::Shop::WipeStock);
	if (network::command::Shop::Gold == type)
		m_player.set_gold(m_player.gold() + num);
	else if (network::command::Shop::Diamond == type)
		m_player.set_diamond(m_player.diamond() + num);
	else
		m_player.set_wipestock(m_player.wipestock() + num);

	//后面把加钱的动作写进日志服务器
//	INFO("加钱的类型:%u,数量:%u,动作：%s", type, num, MoneyActionLog[action]);
}

bool GamePlayer::subMoney(network::command::Shop::CurrencyType type, const uint32 num, MoneyAction action)
{
//	LogCheckCondition(checkMoney(type, num), false, "金钱或钻石数量不足！");
	if (network::command::Shop::Gold == type)
		m_player.set_gold(m_player.gold() - num);
	else
		m_player.set_diamond(m_player.diamond() - num);

	//后面把加钱的动作写进日志服务器
//	INFO("扣钱的类型:%u,数量:%u,动作：%s", type, num, MoneyActionLog[action]);
	return true;
}


void GamePlayer::addAirPlane(uint32 id, bool isBuy)
{
	Airplane *air = m_player.add_airplanelist();
	air->set_id(id);
	if (isBuy) {
		m_player.set_currentairplane(id);
	}

	
	uint32 position_num = TableManager::getInstance().getTable("PlaneProperty")->asInt(id, "position_number");
	if (position_num != IMPOSSIBLE_RETURN)
	{
		switch (position_num)
		{
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
}
