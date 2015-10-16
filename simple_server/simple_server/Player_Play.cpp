#include "Player.h"
#include "TableManager.h"
#include "GameLogic.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace network::command;
using namespace std;

map<uint32, vector<uint32>> GamePlayer::sm_fbRewardScoreJudge;
map<uint32, vector<uint32>> GamePlayer::sm_dropOutVector;

void GamePlayer::startGame(Play::StartGame_CS& msg) {
	auto wls_ptr = TableManager::getInstance().getTable("WayLevelStage");
	int fb_id = wls_ptr->asInt(msg.fbid(), "id");
	if (msg.fbid() != fb_id) {
		printf("客户端传来的副本ID：%u非法！", msg.fbid());
		return ProtocolReturn(msg, Play::StartGame_CS::PARAMETER, CMSGResStartGame_CS);
	}
	if (m_player_status.playing() == true) {
		printf("正在游戏中, %s", m_player.name().c_str());
		return ProtocolReturn(msg, Play::StartGame_CS::PLAYING, CMSGResStartGame_CS);
	}

	if (bagHasFull()) {
		//		MDEBUG("背包已满，不能开始游戏");
		return ProtocolReturn(msg, Play::StartGame_CS::NOSPACE, CMSGResStartGame_CS);
	}

	auto pilot = getPilot(msg.pilotid());
	auto air = getPlane(msg.airplaneid());

	if (pilot == nullptr || air == nullptr) {
		printf("进入关卡:%u失败，角色和飞机不存在！", msg.fbid());
		return ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, CMSGResStartGame_CS);
	}

	auto fb_type = wls_ptr->asInt(msg.fbid(), "type");

	//检测副本的进入条件
	if (static_cast<int>(FbType::FbType_Endless) == fb_type || static_cast<int>(FbType::FbType_Pvp) == fb_type) {
		if (!checkEnterEndlessFb(msg))
			return;
	}
	else if (static_cast<int>(FbType::FbType_Wipe) == msg.fbtype() && static_cast<int>(FbType::FbType_Nomal) == fb_type) {
		if (!checkWipeFb(msg))
			return;
	}
	else if (static_cast<int>(FbType::FbType_Nomal) == fb_type) {
		if (!checkEnterNormalFb(msg))
			return;
	}
	else {
		//		MERROR("非法的副本类型:%u", fb_type);
		return ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, CMSGResStartGame_CS);
	}

	m_player_status.set_playing(true);

	m_player.set_currentpilot(msg.pilotid());
	m_player.set_currentairplane(msg.airplaneid());

	//	subPilotFriendliness(pilot, PlayerInitManager::getInstance().asUInt(0, "friendliness_drop_game"));

	sendPlayerInfo();
	//if (FbType_Wipe == msg.fbtype() && FbType_Nomal == fb_type)//扫荡模式必成功直接结算
	//{
	//	Play::EndGame_CS emsg;
	//	emsg.set_fbid(msg.fbid());
	//	emsg.set_fbtype(FbType_Wipe);
	//	obtainItemFromWipeFb(emsg);
	//	m_player_status.set_playing(false);
	//	return ProtocolReturn(emsg, Play::EndGame_CS::SUCCESS, network::command::CMSGResEndGame_CS);
	//}

	return ProtocolReturn(msg, Play::StartGame_CS::SUCCESS, CMSGResStartGame_CS);
}

void GamePlayer::endGame(Play::EndGame_CS& msg) {
	if (m_player_status.playing() == false) {
		printf("没有正在进行的游戏, %s", m_player.name().c_str());
		return ProtocolReturn(msg, Play::EndGame_CS::NOPLAYING, CMSGResEndGame_CS);
	}
	//此处有个判断是否作弊的接口
	auto wls_ptr = TableManager::getInstance().getTable("WayLevelStage");
	auto tmp_id = wls_ptr->asInt(msg.fbid(), "id");
	if (msg.fbid() != tmp_id)//防止客户端传入参数错误
	{
		printf("客户端传入的无限副本唯一标示符错误:%u, %d", msg.fbid(), tmp_id);

		m_player_status.set_playing(false);
		return ProtocolReturn(msg, Play::EndGame_CS::UNKNOWN, CMSGResEndGame_CS);
	}

	auto type = static_cast<FbType>(wls_ptr->asInt(msg.fbid(), "type"));
	if ((FbType::FbType_Nomal == type) && (Play::EndGame_CS::FAIL == msg.result())) {
		printf("====>玩家闯关:%u--失败======", msg.fbid());
		m_player_status.set_playing(false);
		msg.set_fbtype(static_cast<int>(type));
		return ProtocolReturn(msg, Play::EndGame_CS::FAIL, CMSGResEndGame_CS);
	}

	//-----------------副本的结算------------------
	auto tmp_score = msg.score();
	auto m_add_score = 0;
	auto m_add_exp = 0;

	if (FbType::FbType_Endless == type) {
		obtainItem(msg, FbType::FbType_Endless);
		// 如果更新了记录，则更新
		// 统计无尽模式
		if (tmp_score > m_player.weekrecord().score()) {
			m_player.mutable_weekrecord()->set_weekindex(GameLogic::getLogicInstance()->getWeek());
			m_player.mutable_weekrecord()->set_score(tmp_score);
			m_player.mutable_weekrecord()->set_time(GameLogic::m_current_time);

			if (tmp_score > m_player.bestrecord().record().score()) {
				auto record = new WeekRecord(m_player.weekrecord());
				m_player.mutable_bestrecord()->set_allocated_record(record);
			}
		}

		if (tmp_score > m_player.totalstatistics().endlesstopscore())
			m_player.mutable_totalstatistics()->set_endlesstopscore(tmp_score);
		m_player.mutable_totalstatistics()->set_endlesstimes(m_player.totalstatistics().endlesstimes() + 1);
		m_player.mutable_dailystatistics()->set_endlesstimes(m_player.dailystatistics().endlesstimes() + 1);
	}
	else if (FbType::FbType_Pvp == type) {
		obtainItem(msg, FbType::FbType_Pvp);
	}
	else if (FbType::FbType_Nomal == type) {
		obtainItemFromNormalFb(msg);
	}
	else {
		printf("====非法的副本类型:%u====", type);
	}

	//宠物加成
	//	int m_pet_add = getPetIdByThisId(msg.petthisid());//若宠物没上阵，proto会默认服务器生成的thisid为0，
	//该函数会检测并出现错误日志，对逻辑没有影响。
	//if (1 == msg.flag() && m_pet_add != (unsigned)-1)
	//{
	//	m_add_score = msg.score();
	//	m_add_exp = msg.exp();
	//	addBasicProByPet(m_pet_add, m_add_score, m_add_exp);
	//	msg.set_exp(msg.exp() + m_add_exp);//这两行代码需要后期优化 
	//	addExp(msg.exp(), AddExpAction_WuJing);
	//}

	m_player_status.set_playing(false);

	sendPlayerInfo();
	checkRanking();
	return ProtocolReturn(msg, Play::EndGame_CS::SUCCESS, CMSGResEndGame_CS);
}

void GamePlayer::continueGame(Play::ContinueGame_CS& msg) {
	if (msg.type() == Play::ContinueGame_CS::FREE) {
		msg.set_result(Play::ContinueGame_CS::SUCCESS);
		msg.set_type(Play::ContinueGame_CS::FREE);
		ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
		m_connection->sendCmdMsg(msg__, msg_size__);

		sendPlayerInfo();
	}
	else if (msg.type() == Play::ContinueGame_CS::BUY) {
		// 这里需要读取最大的买活的配置表格
		if (msg.continuecount() >= 10) {
			msg.set_result(Play::ContinueGame_CS::ERR);
			msg.set_type(Play::ContinueGame_CS::BUY);

			ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
			m_connection->sendCmdMsg(msg__, msg_size__);

			sendPlayerInfo();
			return;
		}

		// 读取表格
		uint32 price = 10;

		if (price <= m_player.diamond()) {
			m_player.set_diamond(m_player.diamond() - price);

			msg.set_result(Play::ContinueGame_CS::SUCCESS);
			msg.set_type(Play::ContinueGame_CS::BUY);
			ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
			m_connection->sendCmdMsg(msg__, msg_size__);

			sendPlayerInfo();
		}
		else {
			msg.set_result(Play::ContinueGame_CS::FAIL);
			msg.set_type(Play::ContinueGame_CS::BUY);

			ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
			m_connection->sendCmdMsg(msg__, msg_size__);

			sendPlayerInfo();
		}
	}
}

void GamePlayer::calRewardPart(Play::EndGame_CS& msg, string& grade) const {
	auto score = msg.score();
	auto id = msg.fbid();
	//	INFO("客户端发来的副本%u的分数为%u", id, msg.score());

	if (sm_fbRewardScoreJudge.empty()) {
		vector<string> str_score;
		vector<uint32> score_vec;
		string tmp_score;
		uint32 m_temp = 0;

		tmp_score = TableManager::getInstance().getTable("WayLevelStage")->asString(id, "grade");
		str_score.clear();
		score_vec.clear();

		split(str_score, tmp_score, boost::is_any_of("|"), boost::token_compress_on);
		for (uint32 i = 0; i < str_score.size(); ++i) {
			boost::trim(str_score[i]);
			m_temp = boost::lexical_cast<uint32>(str_score[i]);
			score_vec.push_back(m_temp);
		}
		sm_fbRewardScoreJudge.insert(make_pair(id, score_vec));
	}

	if (sm_fbRewardScoreJudge.find(id) == sm_fbRewardScoreJudge.end()) {
		//		INFO("传入的副本奖励id有误, %d", id);
		return;
	}

	if (score <= sm_fbRewardScoreJudge[id][0])
		grade = "one";
	else if (score <= sm_fbRewardScoreJudge[id][1] && score > sm_fbRewardScoreJudge[id][0])
		grade = "two";
	else if (score <= sm_fbRewardScoreJudge[id][2] && score > sm_fbRewardScoreJudge[id][1])
		grade = "three";
	else if (score <= sm_fbRewardScoreJudge[id][3] && score > sm_fbRewardScoreJudge[id][2])
		grade = "four";
	else if (score <= sm_fbRewardScoreJudge[id][4] && score > sm_fbRewardScoreJudge[id][3])
		grade = "five";
	else if (score <= sm_fbRewardScoreJudge[id][5] && score > sm_fbRewardScoreJudge[id][4])
		grade = "six";
	else if (score <= sm_fbRewardScoreJudge[id][6] && score > sm_fbRewardScoreJudge[id][5])
		grade = "seven";
	else if (score <= sm_fbRewardScoreJudge[id][7] && score > sm_fbRewardScoreJudge[id][6])
		grade = "eight";
	else if (score <= sm_fbRewardScoreJudge[id][8] && score > sm_fbRewardScoreJudge[id][7])
		grade = "nine";
	else if (score > sm_fbRewardScoreJudge[id][9])
		grade = "ten";
}

void GamePlayer::obtainItem(Play::EndGame_CS& msg, FbType fbType) {
	string m_grade = "";
	msg.set_fbtype(static_cast<int>(fbType));
	calRewardPart(msg, m_grade);

	if (m_grade == "") {
		//		MERROR("当前玩家获得奖励的等级有误，为空%s", m_grade.c_str());
		return;
	}

	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");
	auto rtptr = TableManager::getInstance().getTable("RewardTable");
	uint32 id = wlsptr->asInt(msg.fbid(), m_grade.c_str());
	uint32 tmpid = rtptr->asInt(id, "id");
	if (tmpid != id) {
		//		MERROR("WayLevelStage表格中填写的奖励表格行错误, %d", id);
		return;
	}

	uint32 diamond = rtptr->asInt(id, "diamond");
	uint32 exp = rtptr->asInt(id, "exp");
	uint32 gold = rtptr->asInt(id, "gold");

	if (diamond != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Diamond, diamond, MoneyAction::AddDiamondAction_FbDrop);
		msg.set_diamond(diamond);
	}

	if (exp != IMPOSSIBLE_RETURN) {
		addExp(exp);
	}

	if (gold != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Gold, gold, MoneyAction::AddGoldAction_FbDrop);
		msg.set_gold(gold);
	}
	mutextProDrop(msg, id);
}

void GamePlayer::obtainItemFromNormalFb(Play::EndGame_CS& msg) {
	msg.set_fbtype(static_cast<int>(FbType::FbType_Nomal));

	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");
	auto rtptr = TableManager::getInstance().getTable("RewardTable");

	uint32 thisid = wlsptr->asInt(msg.fbid(), "one");

	uint32 tmpid = rtptr->asInt(thisid, "id");
	if (tmpid != thisid) {
		//		MERROR("WayLevelStage表格中填写的奖励表格行错误, %d", thisid);
		return;
	}

	uint32 diamond = rtptr->asInt(thisid, "diamond");
	uint32 exp = rtptr->asInt(thisid, "exp");
	uint32 gold = rtptr->asInt(thisid, "gold");

	if (diamond != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Diamond, diamond, MoneyAction::AddDiamondScoreReward);
		msg.set_diamond(diamond);
	}

	if (exp != IMPOSSIBLE_RETURN) {
		addExp(exp);
	}

	if (gold != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Gold, gold, MoneyAction::AddGoldAction_FbDrop);
		msg.set_gold(gold);
	}

	mutextProDrop(msg, thisid);

	//关卡信息存储
	auto n_normalfbstatistics = m_player.mutable_normalfbstatistics();
	auto n_norfbsta = n_normalfbstatistics->begin();
	uint32 m_groupid = wlsptr->asInt(msg.fbid(), "groupid");
	uint32 m_wipetimes = wlsptr->asInt(msg.fbid(), "times");
	uint32 m_star = wlsptr->asInt(msg.fbid(), "starlevel");

	auto m_group_one = true;
	auto m_group_two = false;
	auto m_group_three = false;
	auto m_old_fb = false;
	uint32 m_three_i = 0;
	uint32 m_two_i = 0;
	uint32 m_three_wipetimes = 0;
	uint32 m_two_wipetimes = 0;
	//下面的代码中如果数据存储比较稳定，则要在if语句中加入break,不需要遍历完整。
	while (n_norfbsta != n_normalfbstatistics->end()) {
		if (msg.fbid() == n_norfbsta->id())//老关卡直接退出，不更新扫荡次数
		{
			m_old_fb = true;
			break;
		}
		++n_norfbsta;
	}
	if (n_norfbsta == n_normalfbstatistics->end()) {
		n_norfbsta = n_normalfbstatistics->begin();
		while (n_norfbsta != n_normalfbstatistics->end()) {
			if (m_groupid == n_norfbsta->groupid() && (3 == m_star))//查找2星副本和1星副本的组ID
			{
				m_three_wipetimes = n_norfbsta->wipetimes();
				m_three_i++;
				if (m_three_i > 2) {
					//					MERROR("====>数据存储错误，大小不应该为:%u,最多为2", m_three_i);
					m_group_three = false;
					break;
				}
				else
					m_group_three = true;
			}
			if (m_groupid == n_norfbsta->groupid() && (2 == m_star))//查找1星副本的组ID
			{
				m_two_wipetimes = n_norfbsta->wipetimes();
				m_two_i++;
				if (m_two_i > 1) {
					//					MERROR("====>数据存储错误，大小不应该为:%u,最多为1", m_two_i);
					m_group_two = false;
					break;
				}
				else
					m_group_two = true;
			}
			if (m_groupid == n_norfbsta->groupid() && (1 == m_star)) {
				//				MERROR("====>数据存储错误,攻打1星副本时，保存的数据中不可能存在该1星副本的组ID:%u", m_groupid);
				m_group_one = false;
				break;
			}
			++n_norfbsta;
		}
	}
	if (!m_old_fb && m_group_three)//三星关卡存储
	{
		NormalFbStatistics* m_normalfb;
		m_normalfb = m_player.add_normalfbstatistics();
		m_normalfb->set_id(msg.fbid());
		m_normalfb->set_wipetimes(m_three_wipetimes);
		m_normalfb->set_groupid(m_groupid);
	}
	if (!m_old_fb && m_group_two)//二星关卡存储
	{
		NormalFbStatistics* n_normalfb;
		n_normalfb = m_player.add_normalfbstatistics();
		n_normalfb->set_id(msg.fbid());
		n_normalfb->set_wipetimes(m_two_wipetimes);
		n_normalfb->set_groupid(m_groupid);
	}
	if (!m_old_fb && m_group_one && 1 == m_star)//一星关卡存储
	{
		NormalFbStatistics* j_normalfb;
		j_normalfb = m_player.add_normalfbstatistics();
		j_normalfb->set_id(msg.fbid());
		j_normalfb->set_wipetimes(m_wipetimes);
		j_normalfb->set_groupid(m_groupid);
	}
}

void GamePlayer::obtainItemFromWipeFb(Play::EndGame_CS& msg) {
	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");
	auto rtptr = TableManager::getInstance().getTable("RewardTable");

	msg.set_fbtype(static_cast<int>(FbType::FbType_Wipe));
	uint32 id = wlsptr->asInt(msg.fbid(), "one");

	uint32 tmpid = rtptr->asInt(id, "id");
	if (tmpid != id) {
		//		MERROR("WayLevelStage表格中填写的奖励表格行错误, %d", id);
		return;
	}

	uint32 diamond = rtptr->asInt(id, "diamond");
	uint32 exp = rtptr->asInt(id, "exp");
	uint32 gold = rtptr->asInt(id, "gold");

	if (diamond != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Diamond, diamond, MoneyAction::AddDiamondScoreReward);
		msg.set_diamond(diamond);
	}

	if (exp != IMPOSSIBLE_RETURN) {
		addExp(exp);
	}

	if (gold != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Gold, gold, MoneyAction::AddGoldAction_FbDrop);
		msg.set_gold(gold);
	}

	mutextProDrop(msg, id);
}

bool GamePlayer::checkEnterNormalFb(Play::StartGame_CS& msg) {
	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");

	//体力检测
	uint32 m_energy = wlsptr->asInt(msg.fbid(), "energy");
	if (m_player.energy() < m_energy) {
		//		MDEBUG("进入关卡:%u的体力不足！", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ENERGY, CMSGResStartGame_CS);
		return false;
	}
	//道具检测
	uint32 m_item = wlsptr->asInt(msg.fbid(), "item");
	if (m_player.itemnum() < m_item && m_item != IMPOSSIBLE_RETURN) {
		//		MDEBUG("进入关卡:%u的道具不足!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ITEM, CMSGResStartGame_CS);
		return false;
	}
	//等级检测
	uint32 m_lev = wlsptr->asInt(msg.fbid(), "lev");
	if (m_player.level() < m_lev) {
		//		MDEBUG("进入关卡:%u的等级不够!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::LEVEL, CMSGResStartGame_CS);
		return false;
	}
	//判断前置关卡是否通过
	auto m_normalfbstatistics = m_player.mutable_normalfbstatistics();
	auto it_norfbsta = m_normalfbstatistics->begin();

	uint32 pre_fb_id = wlsptr->asInt(msg.fbid(), "pre_id");
	auto m_fbid = msg.fbid();
	auto m_pre_flag = false;

	if (pre_fb_id != IMPOSSIBLE_RETURN) {
		while (it_norfbsta != m_normalfbstatistics->end()) {
			if (m_fbid == it_norfbsta->id())//老关卡
			{
				break;
			}
			if (pre_fb_id == it_norfbsta->id())//前置关卡存在
			{
				m_pre_flag = true;
			}
			++it_norfbsta;
		}
		if (it_norfbsta == m_normalfbstatistics->end() && !m_pre_flag)//新关卡,且前置关卡未过
		{

			//			MDEBUG("进入关卡:%u失败，前置关卡:%u未通过！", msg.fbid(), pre_fb_id);
			ProtocolReturn(msg, Play::StartGame_CS::PREFBNOPASS, CMSGResStartGame_CS);
			return false;
		}
	}
	else//第一个关卡
	{
		//		INFO("====>该关卡:%u,是第一个关卡!", msg.fbid());
	}
	//扣体力
	subEnergy(m_energy);
	//扣道具
	m_player.set_itemnum(m_player.itemnum() - m_item);
	return true;
}

bool GamePlayer::checkEnterEndlessFb(Play::StartGame_CS& msg) {
	uint32 fb_energy;
	uint32 m_lev;
	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");

	//体力检测
	fb_energy = wlsptr->asInt(msg.fbid(), "energy");
	if (m_player.energy() < fb_energy) {
		//		MDEBUG("进入关卡:%u失败，体力不够！", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ENERGY, CMSGResStartGame_CS);
		return false;
	}
	//等级检测
	m_lev = wlsptr->asInt(msg.fbid(), "lev");
	if (m_player.level() < m_lev) {
		//		MDEBUG("进入关卡:%u的等级不够!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::LEVEL, CMSGResStartGame_CS);
		return false;
	}
	//扣体力
	subEnergy(fb_energy);
	return true;
}

bool GamePlayer::checkWipeFb(Play::StartGame_CS& msg) {
	//关卡类型检测
	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");
	uint32 m_type = wlsptr->asInt(msg.fbid(), "type");
	if (m_type != static_cast<int>(FbType::FbType_Nomal)) {
		//		MERROR("该关卡：%u不为普通关卡，不能被扫荡!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, CMSGResStartGame_CS);
		return false;
	}
	//该关卡是否已经通过,且扫荡次数是否够
	auto normalfbstatistics = m_player.mutable_normalfbstatistics();
	auto it_norfbsta = normalfbstatistics->begin();
	while (it_norfbsta != normalfbstatistics->end()) {
		if (it_norfbsta->id() == msg.fbid())
			break;
		++it_norfbsta;
	}
	if (it_norfbsta == normalfbstatistics->end()) {
		//		MDEBUG("该关卡:%u没有被通关，不能被扫荡！", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::FBNOPASS, CMSGResStartGame_CS);
		return false;
	}
	//扫荡次数检测
	uint32 m_times = wlsptr->asInt(msg.fbid(), "times");
	for (int i = 0; i < m_player.normalfbstatistics_size(); ++i) {
		if (m_player.normalfbstatistics(i).id() == msg.fbid() && (m_player.normalfbstatistics(i).wipetimes() < 1 || m_player.normalfbstatistics(i).wipetimes() > m_times)) {
			//			MDEBUG("该关卡:%u的剩余次数为0或者保存的剩余扫荡次数非法，不能被扫荡！", msg.fbid());
			ProtocolReturn(msg, Play::StartGame_CS::NOWIPETIMES, CMSGResStartGame_CS);
			return false;
		}
	}
	//扫荡券的数量检测
	if (m_player.wipestock() < 1) {
		//		MDEBUG("该关卡:%u需要的扫荡券不足，不能被扫荡！", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::NOWIPESTOCK, CMSGResStartGame_CS);
		return false;
	}
	//体力检测
	uint32 m_energy = wlsptr->asInt(msg.fbid(), "energy");
	if (m_player.energy() < m_energy) {
		//		MDEBUG("进入关卡:%u的体力不足！", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ENERGY, CMSGResStartGame_CS);
		return false;
	}
	//道具检测
	uint32 m_item = wlsptr->asInt(msg.fbid(), "item");
	if (m_player.itemnum() < m_item && m_item != IMPOSSIBLE_RETURN) {
		//		MDEBUG("进入关卡:%u的道具不足!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ITEM, CMSGResStartGame_CS);
		return false;
	}
	//等级检测
	uint32 m_lev = wlsptr->asInt(msg.fbid(), "lev");
	if (m_player.level() < m_lev) {
		//		MDEBUG("进入关卡:%u的等级不够!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::LEVEL, CMSGResStartGame_CS);
		return false;
	}
	//扣除扫荡次数----------后期要对下面的遍历代码进行优化
	uint32 m_group = wlsptr->asInt(msg.fbid(), "groupid");
	if (it_norfbsta->groupid() != m_group) {
		//		MDEBUG("进入关卡:%u时，服务器存储的关卡组id:%u与策划表中对应的关卡组id:%u不相同!", msg.fbid(), it_norfbsta->groupid(), m_group);
		ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, CMSGResStartGame_CS);
		return false;
	}
	it_norfbsta = normalfbstatistics->begin();
	uint32 i = 0;
	while (it_norfbsta != normalfbstatistics->end() && i < 3) {
		if (it_norfbsta->groupid() == m_group) {
			it_norfbsta->wipetimes() > 1 ? it_norfbsta->set_wipetimes(it_norfbsta->wipetimes() - 1) : it_norfbsta->set_wipetimes(0);
			i++;
		}
		++it_norfbsta;
	}
	//扣除扫荡券的数量
	subWipeStock(1);
	//扣除体力
	subEnergy(m_energy);
	//扣除道具
	subFbUnlockItem(m_item);
	return true;
}

void GamePlayer::independProbDrop(Play::EndGame_CS& msg, uint32 id) {
	uint32 m_pro;
	uint32 m_material_id;
	uint32 m_material_num;
	auto rtptr = TableManager::getInstance().getTable("RewardTable");

	m_pro = rtptr->asInt(id, "pro1");
	if (Utility::selectByRegion(m_pro, 10000)) {
		m_material_id = rtptr->asInt(id, "materi1");
		m_material_num = rtptr->asInt(id, "materi1num");
		auto m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_material_id);
		m_reward->set_materialnum(m_material_num);
		addItemToBag(m_material_id, m_material_num);
	}
	else {
		m_material_id = rtptr->asInt(id, "materi1");
		printf("物品%u没有落在掉落的概率内!", m_material_id);
	}
	m_pro = rtptr->asInt(id, "pro2");
	if (Utility::selectByRegion(m_pro, 10000)) {
		m_material_id = rtptr->asInt(id, "materi2");
		m_material_num = rtptr->asInt(id, "materi2num");
		auto m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_material_id);
		m_reward->set_materialnum(m_material_num);
		addItemToBag(m_material_id, m_material_num);
	}
	else {
		m_material_id = rtptr->asInt(id, "materi2");
		printf("物品%u没有落在掉落的概率内!", m_material_id);
	}
	m_pro = rtptr->asInt(id, "pro3");
	if (Utility::selectByRegion(m_pro, 10000)) {
		m_material_id = rtptr->asInt(id, "materi3");
		m_material_num = rtptr->asInt(id, "materi3num");
		auto m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_material_id);
		m_reward->set_materialnum(m_material_num);
		addItemToBag(m_material_id, m_material_num);
	}
	else {
		m_material_id = rtptr->asInt(id, "materi3");
		printf("物品%u没有落在掉落的概率内!", m_material_id);
	}
	m_pro = rtptr->asInt(id, "pro4");
	if (Utility::selectByRegion(m_pro, 10000)) {
		m_material_id = rtptr->asInt(id, "materi4");
		m_material_num = rtptr->asInt(id, "materi4num");
		auto m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_material_id);
		m_reward->set_materialnum(m_material_num);
		addItemToBag(m_material_id, m_material_num);
	}
	else {
		m_material_id = rtptr->asInt(id, "materi4");
		printf("物品%u没有落在掉落的概率内!", m_material_id);
	}
}

void GamePlayer::mutexProbDrop(uint32 dropid, uint32& id, uint32& number) {
	uint32 num;
	if (sm_dropOutVector.empty())
		storageDropOutTable();
	num = Utility::randBetween(0, 10000);

	if (sm_dropOutVector.find(dropid) == sm_dropOutVector.end()) {
		printf("传入的dropid有误, %d", dropid);
		return;
	}

	auto doptr = TableManager::getInstance().getTable("DropOut");
	if (sm_dropOutVector[dropid][9] == 10000) {
		if (num >= 0 && num <= sm_dropOutVector[dropid][0]) {
			id = doptr->asInt(dropid, "item1");
			number = doptr->asInt(dropid, "itemnum1");
		}
		else if (num >= sm_dropOutVector[dropid][0] && num < sm_dropOutVector[dropid][1]) {
			id = doptr->asInt(dropid, "item2");
			number = doptr->asInt(dropid, "itemnum2");
		}
		else if (num >= sm_dropOutVector[dropid][1] && num < sm_dropOutVector[dropid][2]) {
			id = doptr->asInt(dropid, "item3");
			number = doptr->asInt(dropid, "itemnum3");
		}
		else if (num >= sm_dropOutVector[dropid][2] && num < sm_dropOutVector[dropid][3]) {
			id = doptr->asInt(dropid, "item4");
			number = doptr->asInt(dropid, "itemnum4");
		}
		else if (num >= sm_dropOutVector[dropid][3] && num < sm_dropOutVector[dropid][4]) {
			id = doptr->asInt(dropid, "item5");
			number = doptr->asInt(dropid, "itemnum5");
		}
		else if (num >= sm_dropOutVector[dropid][4] && num < sm_dropOutVector[dropid][5]) {
			id = doptr->asInt(dropid, "item6");
			number = doptr->asInt(dropid, "itemnum6");
		}
		else if (num >= sm_dropOutVector[dropid][5] && num < sm_dropOutVector[dropid][6]) {
			id = doptr->asInt(dropid, "item7");
			number = doptr->asInt(dropid, "itemnum7");
		}
		else if (num >= sm_dropOutVector[dropid][6] && num < sm_dropOutVector[dropid][7]) {
			id = doptr->asInt(dropid, "item8");
			number = doptr->asInt(dropid, "itemnum8");
		}
		else if (num >= sm_dropOutVector[dropid][7] && num < sm_dropOutVector[dropid][8]) {
			id = doptr->asInt(dropid, "item9");
			number = doptr->asInt(dropid, "itemnum9");
		}
		else if (num >= sm_dropOutVector[dropid][8] && num < sm_dropOutVector[dropid][9]) {
			id = doptr->asInt(dropid, "item10");
			number = doptr->asInt(dropid, "itemnum10");
		}
	}
	else {
		//		MERROR("DropOutManager配表错误,概率和大于10000！");
	}
}

void GamePlayer::mutextProDrop(Play::EndGame_CS& msg, uint32 groupid) {
	auto rtptr = TableManager::getInstance().getTable("RewardTable");
	auto doptr = TableManager::getInstance().getTable("DropOut");
	uint32 m_dropid1 = rtptr->asInt(groupid, "dropid1");
	uint32 m_dropid2 = rtptr->asInt(groupid, "dropid2");
	uint32 m_dropid3 = rtptr->asInt(groupid, "dropid3");
	uint32 m_dropid4 = rtptr->asInt(groupid, "dropid4");
	uint32 m_mintimes1 = doptr->asInt(m_dropid1, "mintimes");
	uint32 m_maxtimes1 = doptr->asInt(m_dropid1, "maxtimes");
	uint32 m_mintimes2 = doptr->asInt(m_dropid2, "mintimes");
	uint32 m_maxtimes2 = doptr->asInt(m_dropid2, "maxtimes");
	uint32 m_mintimes3 = doptr->asInt(m_dropid3, "mintimes");
	uint32 m_maxtimes3 = doptr->asInt(m_dropid3, "maxtimes");
	uint32 m_mintimes4 = doptr->asInt(m_dropid4, "mintimes");
	uint32 m_maxtimes4 = doptr->asInt(m_dropid4, "maxtimes");
	uint32 m_pro1 = Utility::randBetween(m_mintimes1, m_maxtimes1);
	uint32 m_pro2 = Utility::randBetween(m_mintimes2, m_maxtimes2);
	uint32 m_pro3 = Utility::randBetween(m_mintimes3, m_maxtimes3);
	uint32 m_pro4 = Utility::randBetween(m_mintimes4, m_maxtimes4);
	uint32 m_id = 0;
	uint32 m_num = 0;
	for (uint32 i = 0; i < m_pro1 && m_dropid1 != IMPOSSIBLE_RETURN; ++i) {
		mutexProbDrop(m_dropid1, m_id, m_num);
		auto m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_id);
		m_reward->set_materialnum(m_num);
		addItemToBag(m_id, m_num);
		//		INFO("关卡掉落物品ID:%u,物品数量:%u", m_id, m_num);
	}
	for (uint32 j = 0; j < m_pro2 && m_dropid2 != IMPOSSIBLE_RETURN; ++j) {
		mutexProbDrop(m_dropid2, m_id, m_num);
		auto m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_id);
		m_reward->set_materialnum(m_num);
		addItemToBag(m_id, m_num);
		//		INFO("关卡掉落物品ID:%u,物品数量:%u", m_id, m_num);
	}
	for (uint32 k = 0; k < m_pro3 && m_dropid3 != IMPOSSIBLE_RETURN; ++k) {
		mutexProbDrop(m_dropid3, m_id, m_num);
		auto m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_id);
		m_reward->set_materialnum(m_num);
		addItemToBag(m_id, m_num);
		//		INFO("关卡掉落物品ID:%u,物品数量:%u", m_id, m_num);
	}
	for (uint32 l = 0; l < m_pro4 && m_dropid4 != IMPOSSIBLE_RETURN; ++l) {
		mutexProbDrop(m_dropid4, m_id, m_num);
		auto m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_id);
		m_reward->set_materialnum(m_num);
		addItemToBag(m_id, m_num);
		//		INFO("关卡掉落物品ID:%u,物品数量:%u", m_id, m_num);
	}
}

void GamePlayer::storageDropOutTable() const {
	vector<uint32> rand_vec;
	//std::vector<uint32> idlist = TaskInfoManager::getInstance().getTableId();
	auto doptr = TableManager::getInstance().getTable("DropOut");
	auto idlist = TableManager::getInstance().getTable("DropOut")->getTableIdList();
	for (uint32 j = 0; j < idlist.size(); ++j) {
		uint32 i = idlist[j]; // 获取ID
		uint32 tmp_id = doptr->asInt(i, "dropid");
		rand_vec.clear();
		uint32 rand1 = doptr->asInt(i, "rand1");
		rand_vec.push_back(rand1);
		uint32 rand2 = doptr->asInt(i, "rand2");
		rand_vec.push_back(rand1 + rand2);
		uint32 rand3 = doptr->asInt(i, "rand3");
		rand_vec.push_back(rand1 + rand2 + rand3);
		uint32 rand4 = doptr->asInt(i, "rand4");
		rand_vec.push_back(rand1 + rand2 + rand3 + rand4);
		uint32 rand5 = doptr->asInt(i, "rand5");
		rand_vec.push_back(rand1 + rand2 + rand3 + rand4 + rand5);
		uint32 rand6 = doptr->asInt(i, "rand6");
		rand_vec.push_back(rand1 + rand2 + rand3 + rand4 + rand5 + rand6);
		uint32 rand7 = doptr->asInt(i, "rand7");
		rand_vec.push_back(rand1 + rand2 + rand3 + rand4 + rand5 + rand6 + rand7);
		uint32 rand8 = doptr->asInt(i, "rand8");
		rand_vec.push_back(rand1 + rand2 + rand3 + rand4 + rand5 + rand6 + rand7 + rand8);
		uint32 rand9 = doptr->asInt(i, "rand9");
		rand_vec.push_back(rand1 + rand2 + rand3 + rand4 + rand5 + rand6 + rand7 + rand8 + rand9);
		uint32 rand10 = doptr->asInt(i, "rand10");
		rand_vec.push_back(rand1 + rand2 + rand3 + rand4 + rand5 + rand6 + rand7 + rand8 + rand9 + rand10);

		sm_dropOutVector.insert(make_pair(tmp_id, rand_vec));
	}
}

void GamePlayer::updateWipeTimes() {
	for (auto i = 0; i < m_player.normalfbstatistics_size(); ++i) {
		NormalFbStatistics* m_norfb;
		m_norfb = m_player.mutable_normalfbstatistics(i);
		m_norfb->set_wipetimes(4);
	}
}
