#include "Player.h"
#include "TableManager.h"
#include "GameLogic.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace network::command;
using namespace std;

map<uint32, vector<uint32>> GamePlayer::sm_fbRewardScoreJudge;
map<uint32, vector<uint32>> GamePlayer::sm_dropOutVector;

void GamePlayer::startGame(network::command::Play::StartGame_CS& msg) {
	auto wls_ptr = TableManager::getInstance().getTable("WayLevelStage");
	int fb_id = wls_ptr->asInt(msg.fbid(), "id");
	if (msg.fbid() != fb_id) {
		printf("�ͻ��˴����ĸ���ID��%u�Ƿ���", msg.fbid());
		return ProtocolReturn(msg, Play::StartGame_CS::PARAMETER, network::command::CMSGResStartGame_CS);
	}
	if (m_player_status.playing() == true) {
		printf("������Ϸ��, %s", m_player.name().c_str());
		return ProtocolReturn(msg, Play::StartGame_CS::PLAYING, network::command::CMSGResStartGame_CS);
	}

	if (bagHasFull()) {
		//		MDEBUG("�������������ܿ�ʼ��Ϸ");
		return ProtocolReturn(msg, Play::StartGame_CS::NOSPACE, network::command::CMSGResStartGame_CS);
	}

	Pilot* pilot = getPilot(msg.pilotid());
	Airplane* air = getPlane(msg.airplaneid());

	if (pilot == nullptr || air == nullptr) {
		printf("����ؿ�:%uʧ�ܣ���ɫ�ͷɻ������ڣ�", msg.fbid());
		return ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, network::command::CMSGResStartGame_CS);
	}

	int fb_type = wls_ptr->asInt(msg.fbid(), "type");

	//��⸱���Ľ�������
	if (FbType_Endless == fb_type || FbType_Pvp == fb_type)
	{
		if (!checkEnterEndlessFb(msg))
			return;
	}
	else if (FbType_Wipe == msg.fbtype() && FbType_Nomal == fb_type)
	{
		if (!checkWipeFb(msg))
			return;
	}
	else if (FbType_Nomal == fb_type)
	{
		if (!checkEnterNormalFb(msg))
			return;
	}
	else
	{
//		MERROR("�Ƿ��ĸ�������:%u", fb_type);
		return ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, network::command::CMSGResStartGame_CS);
	}

	m_player_status.set_playing(true);

	m_player.set_currentpilot(msg.pilotid());
	m_player.set_currentairplane(msg.airplaneid());

	//	subPilotFriendliness(pilot, PlayerInitManager::getInstance().asUInt(0, "friendliness_drop_game"));

	sendPlayerInfo();
	//if (FbType_Wipe == msg.fbtype() && FbType_Nomal == fb_type)//ɨ��ģʽ�سɹ�ֱ�ӽ���
	//{
	//	Play::EndGame_CS emsg;
	//	emsg.set_fbid(msg.fbid());
	//	emsg.set_fbtype(FbType_Wipe);
	//	obtainItemFromWipeFb(emsg);
	//	m_player_status.set_playing(false);
	//	return ProtocolReturn(emsg, Play::EndGame_CS::SUCCESS, network::command::CMSGResEndGame_CS);
	//}

	return ProtocolReturn(msg, Play::StartGame_CS::SUCCESS, network::command::CMSGResStartGame_CS);
}

void GamePlayer::endGame(network::command::Play::EndGame_CS& msg) {
	if (m_player_status.playing() == false) {
		printf("û�����ڽ��е���Ϸ, %s", m_player.name().c_str());
		return ProtocolReturn(msg, Play::EndGame_CS::NOPLAYING, network::command::CMSGResEndGame_CS);
	}
	//�˴��и��ж��Ƿ����׵Ľӿ�
	auto wls_ptr = TableManager::getInstance().getTable("WayLevelStage");
	int tmp_id = wls_ptr->asInt(msg.fbid(), "id");
	if (msg.fbid() != tmp_id)//��ֹ�ͻ��˴����������
	{
		printf("�ͻ��˴�������޸���Ψһ��ʾ������:%u, %d", msg.fbid(), tmp_id);

		m_player_status.set_playing(false);
		return ProtocolReturn(msg, Play::EndGame_CS::UNKNOWN, network::command::CMSGResEndGame_CS);
	}

	int type = wls_ptr->asInt(msg.fbid(), "type");
	if ((FbType_Nomal == type) && (Play::EndGame_CS::FAIL == msg.result())) {
		printf("====>��Ҵ���:%u--ʧ��======", msg.fbid());
		m_player_status.set_playing(false);
		msg.set_fbtype(type);
		return ProtocolReturn(msg, Play::EndGame_CS::FAIL, network::command::CMSGResEndGame_CS);
	}

	//-----------------�����Ľ���------------------
	uint32 tmp_score = msg.score();
	int m_add_score = 0;
	int m_add_exp = 0;

	if (FbType_Endless == type) {
		obtainItem(msg, FbType_Endless);
		// ��������˼�¼�������
		// ͳ���޾�ģʽ
		if (tmp_score > m_player.weekrecord().score()) {
			m_player.mutable_weekrecord()->set_weekindex(GameLogic::getLogicInstance()->getWeek());
			m_player.mutable_weekrecord()->set_score(tmp_score);
			m_player.mutable_weekrecord()->set_time(GameLogic::m_current_time);

			if (tmp_score > m_player.bestrecord().record().score()) {
				WeekRecord* record = new WeekRecord(m_player.weekrecord());
				m_player.mutable_bestrecord()->set_allocated_record(record);
			}
		}

		if (tmp_score > m_player.totalstatistics().endlesstopscore())
			m_player.mutable_totalstatistics()->set_endlesstopscore(tmp_score);
		m_player.mutable_totalstatistics()->set_endlesstimes(m_player.totalstatistics().endlesstimes() + 1);
		m_player.mutable_dailystatistics()->set_endlesstimes(m_player.dailystatistics().endlesstimes() + 1);
	}
	else if (FbType_Pvp == type) {
		obtainItem(msg, FbType_Pvp);
	}
	else if (FbType_Nomal == type/*&&Play::EndGame_CS::SUCCESS==msg.result()*/) {
		obtainItemFromNormalFb(msg);
	}
	else {
		printf("====�Ƿ��ĸ�������:%u====", type);
	}

	//����ӳ�
	//	int m_pet_add = getPetIdByThisId(msg.petthisid());//������û����proto��Ĭ�Ϸ��������ɵ�thisidΪ0��
	//�ú������Ⲣ���ִ�����־�����߼�û��Ӱ�졣
	//if (1 == msg.flag() && m_pet_add != (unsigned)-1)
	//{
	//	m_add_score = msg.score();
	//	m_add_exp = msg.exp();
	//	addBasicProByPet(m_pet_add, m_add_score, m_add_exp);
	//	msg.set_exp(msg.exp() + m_add_exp);//�����д�����Ҫ�����Ż� 
	//	addExp(msg.exp(), AddExpAction_WuJing);
	//}

	m_player_status.set_playing(false);

	sendPlayerInfo();
	checkRanking();
	return ProtocolReturn(msg, Play::EndGame_CS::SUCCESS, network::command::CMSGResEndGame_CS);
}

void GamePlayer::continueGame(network::command::Play::ContinueGame_CS& msg) {
	if (msg.type() == Play::ContinueGame_CS::FREE) {
		msg.set_result(Play::ContinueGame_CS::SUCCESS);
		msg.set_type(Play::ContinueGame_CS::FREE);
		ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
		m_connection->sendCmdMsg(msg__, msg_size__);

		sendPlayerInfo();
	}
	else if (msg.type() == Play::ContinueGame_CS::BUY) {
		// ������Ҫ��ȡ�����������ñ��
		if (msg.continuecount() >= 10) {
			msg.set_result(Play::ContinueGame_CS::ERR);
			msg.set_type(Play::ContinueGame_CS::BUY);

			ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
			m_connection->sendCmdMsg(msg__, msg_size__);

			sendPlayerInfo();
			return;
		}

		// ��ȡ���
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

void GamePlayer::calRewardPart(network::command::Play::EndGame_CS& msg, std::string& grade) {
	uint32 score = msg.score();
	uint32 id = msg.fbid();
	//	INFO("�ͻ��˷����ĸ���%u�ķ���Ϊ%u", id, msg.score());

	if (sm_fbRewardScoreJudge.empty()) {
		std::vector<std::string> str_score;
		std::vector<uint32> score_vec;
		std::string tmp_score;
		uint32 m_temp = 0;

		tmp_score = TableManager::getInstance().getTable("WayLevelStage")->asString(id, "grade");
		str_score.clear();
		score_vec.clear();

		boost::split(str_score, tmp_score, boost::is_any_of("|"), boost::token_compress_on);
		for (uint32 i = 0; i < str_score.size(); ++i) {
			boost::trim(str_score[i]);
			m_temp = boost::lexical_cast<uint32>(str_score[i]);
			score_vec.push_back(m_temp);
		}
		sm_fbRewardScoreJudge.insert(make_pair(id, score_vec));
	}

	if (sm_fbRewardScoreJudge.find(id) == sm_fbRewardScoreJudge.end()) {
		//		INFO("����ĸ�������id����, %d", id);
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

void GamePlayer::obtainItem(network::command::Play::EndGame_CS& msg, uint32 fbType) {
	std::string m_grade = "";
	msg.set_fbtype(fbType);
	calRewardPart(msg, m_grade);

	if (m_grade == "") {
		//		MERROR("��ǰ��һ�ý����ĵȼ�����Ϊ��%s", m_grade.c_str());
		return;
	}

	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");
	auto rtptr = TableManager::getInstance().getTable("RewardTable");
	uint32 id = wlsptr->asInt(msg.fbid(), m_grade.c_str());
	uint32 tmpid = rtptr->asInt(id, "id");
	if (tmpid != id) {
		//		MERROR("WayLevelStage�������д�Ľ�������д���, %d", id);
		return;
	}

	uint32 diamond = rtptr->asInt(id, "diamond");
	uint32 exp = rtptr->asInt(id, "exp");
	uint32 gold = rtptr->asInt(id, "gold");

	if (diamond != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Diamond, diamond, AddDiamondAction_FbDrop);
		msg.set_diamond(diamond);
	}

	if (exp != IMPOSSIBLE_RETURN) {
		addExp(exp);
	}

	if (gold != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Gold, gold, AddGoldAction_FbDrop);
		msg.set_gold(gold);
	}
	mutextProDrop(msg, id);
}

void GamePlayer::obtainItemFromNormalFb(network::command::Play::EndGame_CS& msg) {
	msg.set_fbtype(FbType_Nomal);

	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");
	auto rtptr = TableManager::getInstance().getTable("RewardTable");

	uint32 thisid = wlsptr->asInt(msg.fbid(), "one");

	uint32 tmpid = rtptr->asInt(thisid, "id");
	if (tmpid != thisid) {
		//		MERROR("WayLevelStage�������д�Ľ�������д���, %d", thisid);
		return;
	}

	uint32 diamond = rtptr->asInt(thisid, "diamond");
	uint32 exp = rtptr->asInt(thisid, "exp");
	uint32 gold = rtptr->asInt(thisid, "gold");

	if (diamond != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Diamond, diamond, AddDiamondScoreReward);
		msg.set_diamond(diamond);
	}

	if (exp != IMPOSSIBLE_RETURN) {
		addExp(exp);
	}

	if (gold != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Gold, gold, AddGoldAction_FbDrop);
		msg.set_gold(gold);
	}

	mutextProDrop(msg, thisid);

	//�ؿ���Ϣ�洢
	auto n_normalfbstatistics = m_player.mutable_normalfbstatistics();
	auto n_norfbsta = n_normalfbstatistics->begin();
	uint32 m_groupid = wlsptr->asInt(msg.fbid(), "groupid");
	uint32 m_wipetimes = wlsptr->asInt(msg.fbid(), "times");
	uint32 m_star = wlsptr->asInt(msg.fbid(), "starlevel");

	bool m_group_one = true;
	bool m_group_two = false;
	bool m_group_three = false;
	bool m_old_fb = false;
	uint32 m_three_i = 0;
	uint32 m_two_i = 0;
	uint32 m_three_wipetimes = 0;
	uint32 m_two_wipetimes = 0;
	//����Ĵ�����������ݴ洢�Ƚ��ȶ�����Ҫ��if����м���break,����Ҫ����������
	while (n_norfbsta != n_normalfbstatistics->end()) {
		if (msg.fbid() == n_norfbsta->id())//�Ϲؿ�ֱ���˳���������ɨ������
		{
			m_old_fb = true;
			break;
		}
		++n_norfbsta;
	}
	if (n_norfbsta == n_normalfbstatistics->end()) {
		n_norfbsta = n_normalfbstatistics->begin();
		while (n_norfbsta != n_normalfbstatistics->end()) {
			if (m_groupid == n_norfbsta->groupid() && (3 == m_star))//����2�Ǹ�����1�Ǹ�������ID
			{
				m_three_wipetimes = n_norfbsta->wipetimes();
				m_three_i++;
				if (m_three_i > 2) {
					//					MERROR("====>���ݴ洢���󣬴�С��Ӧ��Ϊ:%u,���Ϊ2", m_three_i);
					m_group_three = false;
					break;
				}
				else
					m_group_three = true;
			}
			if (m_groupid == n_norfbsta->groupid() && (2 == m_star))//����1�Ǹ�������ID
			{
				m_two_wipetimes = n_norfbsta->wipetimes();
				m_two_i++;
				if (m_two_i > 1) {
					//					MERROR("====>���ݴ洢���󣬴�С��Ӧ��Ϊ:%u,���Ϊ1", m_two_i);
					m_group_two = false;
					break;
				}
				else
					m_group_two = true;
			}
			if (m_groupid == n_norfbsta->groupid() && (1 == m_star)) {
				//				MERROR("====>���ݴ洢����,����1�Ǹ���ʱ������������в����ܴ��ڸ�1�Ǹ�������ID:%u", m_groupid);
				m_group_one = false;
				break;
			}
			++n_norfbsta;
		}
	}
	if (!m_old_fb && m_group_three)//���ǹؿ��洢
	{
		::network::command::NormalFbStatistics* m_normalfb = NULL;
		m_normalfb = m_player.add_normalfbstatistics();
		m_normalfb->set_id(msg.fbid());
		m_normalfb->set_wipetimes(m_three_wipetimes);
		m_normalfb->set_groupid(m_groupid);
	}
	if (!m_old_fb && m_group_two)//���ǹؿ��洢
	{
		::network::command::NormalFbStatistics* n_normalfb = NULL;
		n_normalfb = m_player.add_normalfbstatistics();
		n_normalfb->set_id(msg.fbid());
		n_normalfb->set_wipetimes(m_two_wipetimes);
		n_normalfb->set_groupid(m_groupid);
	}
	if (!m_old_fb && m_group_one && 1 == m_star)//һ�ǹؿ��洢
	{
		::network::command::NormalFbStatistics* j_normalfb = NULL;
		j_normalfb = m_player.add_normalfbstatistics();
		j_normalfb->set_id(msg.fbid());
		j_normalfb->set_wipetimes(m_wipetimes);
		j_normalfb->set_groupid(m_groupid);
	}
}

void GamePlayer::obtainItemFromWipeFb(network::command::Play::EndGame_CS& msg) {
	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");
	auto rtptr = TableManager::getInstance().getTable("RewardTable");

	msg.set_fbtype(FbType_Wipe);
	uint32 id = wlsptr->asInt(msg.fbid(), "one");

	uint32 tmpid = rtptr->asInt(id, "id");
	if (tmpid != id) {
		//		MERROR("WayLevelStage�������д�Ľ�������д���, %d", id);
		return;
	}

	uint32 diamond = rtptr->asInt(id, "diamond");
	uint32 exp = rtptr->asInt(id, "exp");
	uint32 gold = rtptr->asInt(id, "gold");

	if (diamond != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Diamond, diamond, AddDiamondScoreReward);
		msg.set_diamond(diamond);
	}

	if (exp != IMPOSSIBLE_RETURN) {
		addExp(exp);
	}

	if (gold != IMPOSSIBLE_RETURN) {
		addMoney(Shop::Gold, gold, AddGoldAction_FbDrop);
		msg.set_gold(gold);
	}

	mutextProDrop(msg, id);
}

bool GamePlayer::checkEnterNormalFb(network::command::Play::StartGame_CS& msg) {
	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");

	//�������
	uint32 m_energy = wlsptr->asInt(msg.fbid(), "energy");
	if (m_player.energy() < m_energy) {
		//		MDEBUG("����ؿ�:%u���������㣡", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ENERGY, network::command::CMSGResStartGame_CS);
		return false;
	}
	//���߼��
	uint32 m_item = wlsptr->asInt(msg.fbid(), "item");
	if (m_player.itemnum() < m_item && m_item != IMPOSSIBLE_RETURN) {
		//		MDEBUG("����ؿ�:%u�ĵ��߲���!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ITEM, network::command::CMSGResStartGame_CS);
		return false;
	}
	//�ȼ����
	uint32 m_lev = wlsptr->asInt(msg.fbid(), "lev");
	if (m_player.level() < m_lev) {
		//		MDEBUG("����ؿ�:%u�ĵȼ�����!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::LEVEL, network::command::CMSGResStartGame_CS);
		return false;
	}
	//�ж�ǰ�ùؿ��Ƿ�ͨ��
	auto m_normalfbstatistics = m_player.mutable_normalfbstatistics();
	auto it_norfbsta = m_normalfbstatistics->begin();

	uint32 pre_fb_id = wlsptr->asInt(msg.fbid(), "pre_id");
	uint32 m_fbid = msg.fbid();
	bool m_pre_flag = false;

	if (pre_fb_id != IMPOSSIBLE_RETURN) {
		while (it_norfbsta != m_normalfbstatistics->end()) {
			if (m_fbid == it_norfbsta->id())//�Ϲؿ�
			{
				break;
			}
			if (pre_fb_id == it_norfbsta->id())//ǰ�ùؿ�����
			{
				m_pre_flag = true;
			}
			++it_norfbsta;
		}
		if (it_norfbsta == m_normalfbstatistics->end() && !m_pre_flag)//�¹ؿ�,��ǰ�ùؿ�δ��
		{

			//			MDEBUG("����ؿ�:%uʧ�ܣ�ǰ�ùؿ�:%uδͨ����", msg.fbid(), pre_fb_id);
			ProtocolReturn(msg, Play::StartGame_CS::PREFBNOPASS, network::command::CMSGResStartGame_CS);
			return false;
		}
	}
	else//��һ���ؿ�
	{
		//		INFO("====>�ùؿ�:%u,�ǵ�һ���ؿ�!", msg.fbid());
	}
	//������
	subEnergy(m_energy);
	//�۵���
	m_player.set_itemnum(m_player.itemnum() - m_item);
	return true;
}

bool GamePlayer::checkEnterEndlessFb(network::command::Play::StartGame_CS& msg) {
	uint32 fb_energy = 0;
	uint32 m_lev = 0;
	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");

	//�������
	fb_energy = wlsptr->asInt(msg.fbid(), "energy");
	if (m_player.energy() < fb_energy) {
		//		MDEBUG("����ؿ�:%uʧ�ܣ�����������", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ENERGY, network::command::CMSGResStartGame_CS);
		return false;
	}
	//�ȼ����
	m_lev = wlsptr->asInt(msg.fbid(), "lev");
	if (m_player.level() < m_lev) {
		//		MDEBUG("����ؿ�:%u�ĵȼ�����!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::LEVEL, network::command::CMSGResStartGame_CS);
		return false;
	}
	//������
	subEnergy(fb_energy);
	return true;
}

bool GamePlayer::checkWipeFb(network::command::Play::StartGame_CS& msg) {
	//�ؿ����ͼ��
	auto wlsptr = TableManager::getInstance().getTable("WayLevelStage");
	uint32 m_type = wlsptr->asInt(msg.fbid(), "type");
	if (m_type != FbType_Nomal) {
		//		MERROR("�ùؿ���%u��Ϊ��ͨ�ؿ������ܱ�ɨ��!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, network::command::CMSGResStartGame_CS);
		return false;
	}
	//�ùؿ��Ƿ��Ѿ�ͨ��,��ɨ�������Ƿ�
	auto normalfbstatistics = m_player.mutable_normalfbstatistics();
	auto it_norfbsta = normalfbstatistics->begin();
	while (it_norfbsta != normalfbstatistics->end()) {
		if (it_norfbsta->id() == msg.fbid())
			break;
		++it_norfbsta;
	}
	if (it_norfbsta == normalfbstatistics->end()) {
		//		MDEBUG("�ùؿ�:%uû�б�ͨ�أ����ܱ�ɨ����", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::FBNOPASS, network::command::CMSGResStartGame_CS);
		return false;
	}
	//ɨ���������
	uint32 m_times = wlsptr->asInt(msg.fbid(), "times");
	for (int i = 0; i < m_player.normalfbstatistics_size(); ++i) {
		if (m_player.normalfbstatistics(i).id() == msg.fbid() && (m_player.normalfbstatistics(i).wipetimes() < 1 || m_player.normalfbstatistics(i).wipetimes() > m_times)) {
			//			MDEBUG("�ùؿ�:%u��ʣ�����Ϊ0���߱����ʣ��ɨ�������Ƿ������ܱ�ɨ����", msg.fbid());
			ProtocolReturn(msg, Play::StartGame_CS::NOWIPETIMES, network::command::CMSGResStartGame_CS);
			return false;
		}
	}
	//ɨ��ȯ���������
	if (m_player.wipestock() < 1) {
		//		MDEBUG("�ùؿ�:%u��Ҫ��ɨ��ȯ���㣬���ܱ�ɨ����", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::NOWIPESTOCK, network::command::CMSGResStartGame_CS);
		return false;
	}
	//�������
	uint32 m_energy = wlsptr->asInt(msg.fbid(), "energy");
	if (m_player.energy() < m_energy) {
		//		MDEBUG("����ؿ�:%u���������㣡", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ENERGY, network::command::CMSGResStartGame_CS);
		return false;
	}
	//���߼��
	uint32 m_item = wlsptr->asInt(msg.fbid(), "item");
	if (m_player.itemnum() < m_item && m_item != IMPOSSIBLE_RETURN) {
		//		MDEBUG("����ؿ�:%u�ĵ��߲���!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::ITEM, network::command::CMSGResStartGame_CS);
		return false;
	}
	//�ȼ����
	uint32 m_lev = wlsptr->asInt(msg.fbid(), "lev");
	if (m_player.level() < m_lev) {
		//		MDEBUG("����ؿ�:%u�ĵȼ�����!", msg.fbid());
		ProtocolReturn(msg, Play::StartGame_CS::LEVEL, network::command::CMSGResStartGame_CS);
		return false;
	}
	//�۳�ɨ������----------����Ҫ������ı�����������Ż�
	uint32 m_group = wlsptr->asInt(msg.fbid(), "groupid");
	if (it_norfbsta->groupid() != m_group) {
		//		MDEBUG("����ؿ�:%uʱ���������洢�Ĺؿ���id:%u��߻����ж�Ӧ�Ĺؿ���id:%u����ͬ!", msg.fbid(), it_norfbsta->groupid(), m_group);
		ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, network::command::CMSGResStartGame_CS);
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
	//�۳�ɨ��ȯ������
	subWipeStock(1);
	//�۳�����
	subEnergy(m_energy);
	//�۳�����
	subFbUnlockItem(m_item);
	return true;
}

void GamePlayer::independProbDrop(network::command::Play::EndGame_CS& msg, uint32 id) {
	uint32 m_pro = 0;
	uint32 m_material_id = 0;
	uint32 m_material_num = 0;
	auto rtptr = TableManager::getInstance().getTable("RewardTable");

	m_pro = rtptr->asInt(id, "pro1");
	if (Utility::selectByRegion(m_pro, 10000)) {
		m_material_id = rtptr->asInt(id, "materi1");
		m_material_num = rtptr->asInt(id, "materi1num");
		network::command::RewardGoods* m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_material_id);
		m_reward->set_materialnum(m_material_num);
		addItemToBag(m_material_id, m_material_num);
	}
	else {
		m_material_id = rtptr->asInt(id, "materi1");
		printf("��Ʒ%uû�����ڵ���ĸ�����!", m_material_id);
	}
	m_pro = rtptr->asInt(id, "pro2");
	if (Utility::selectByRegion(m_pro, 10000)) {
		m_material_id = rtptr->asInt(id, "materi2");
		m_material_num = rtptr->asInt(id, "materi2num");
		network::command::RewardGoods* m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_material_id);
		m_reward->set_materialnum(m_material_num);
		addItemToBag(m_material_id, m_material_num);
	}
	else {
		m_material_id = rtptr->asInt(id, "materi2");
		printf("��Ʒ%uû�����ڵ���ĸ�����!", m_material_id);
	}
	m_pro = rtptr->asInt(id, "pro3");
	if (Utility::selectByRegion(m_pro, 10000)) {
		m_material_id = rtptr->asInt(id, "materi3");
		m_material_num = rtptr->asInt(id, "materi3num");
		network::command::RewardGoods* m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_material_id);
		m_reward->set_materialnum(m_material_num);
		addItemToBag(m_material_id, m_material_num);
	}
	else {
		m_material_id = rtptr->asInt(id, "materi3");
		printf("��Ʒ%uû�����ڵ���ĸ�����!", m_material_id);
	}
	m_pro = rtptr->asInt(id, "pro4");
	if (Utility::selectByRegion(m_pro, 10000)) {
		m_material_id = rtptr->asInt(id, "materi4");
		m_material_num = rtptr->asInt(id, "materi4num");
		network::command::RewardGoods* m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_material_id);
		m_reward->set_materialnum(m_material_num);
		addItemToBag(m_material_id, m_material_num);
	}
	else {
		m_material_id = rtptr->asInt(id, "materi4");
		printf("��Ʒ%uû�����ڵ���ĸ�����!", m_material_id);
	}
}

void GamePlayer::mutexProbDrop(uint32 dropid, uint32& id, uint32& number) {
	uint32 num = 0;
	if (sm_dropOutVector.empty())
		storageDropOutTable();
	num = Utility::randBetween(0, 10000);

	if (sm_dropOutVector.find(dropid) == sm_dropOutVector.end()) {
		printf("�����dropid����, %d", dropid);
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
		//		MERROR("DropOutManager������,���ʺʹ���10000��");
	}
}

void GamePlayer::mutextProDrop(network::command::Play::EndGame_CS& msg, uint32 groupid) {
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
		network::command::RewardGoods* m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_id);
		m_reward->set_materialnum(m_num);
		addItemToBag(m_id, m_num);
		//		INFO("�ؿ�������ƷID:%u,��Ʒ����:%u", m_id, m_num);
	}
	for (uint32 j = 0; j < m_pro2 && m_dropid2 != IMPOSSIBLE_RETURN; ++j) {
		mutexProbDrop(m_dropid2, m_id, m_num);
		network::command::RewardGoods* m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_id);
		m_reward->set_materialnum(m_num);
		addItemToBag(m_id, m_num);
		//		INFO("�ؿ�������ƷID:%u,��Ʒ����:%u", m_id, m_num);
	}
	for (uint32 k = 0; k < m_pro3 && m_dropid3 != IMPOSSIBLE_RETURN; ++k) {
		mutexProbDrop(m_dropid3, m_id, m_num);
		network::command::RewardGoods* m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_id);
		m_reward->set_materialnum(m_num);
		addItemToBag(m_id, m_num);
		//		INFO("�ؿ�������ƷID:%u,��Ʒ����:%u", m_id, m_num);
	}
	for (uint32 l = 0; l < m_pro4 && m_dropid4 != IMPOSSIBLE_RETURN; ++l) {
		mutexProbDrop(m_dropid4, m_id, m_num);
		network::command::RewardGoods* m_reward = msg.add_rewardgoods();
		m_reward->set_materialid(m_id);
		m_reward->set_materialnum(m_num);
		addItemToBag(m_id, m_num);
		//		INFO("�ؿ�������ƷID:%u,��Ʒ����:%u", m_id, m_num);
	}
}

void GamePlayer::storageDropOutTable() {
	std::vector<uint32> rand_vec;
	//std::vector<uint32> idlist = TaskInfoManager::getInstance().getTableId();
	auto doptr = TableManager::getInstance().getTable("DropOut");
	auto idlist = TableManager::getInstance().getTable("DropOut")->getTableIdList();
	for (uint32 j = 0; j < idlist.size(); ++j) {
		uint32 i = idlist[j]; // ��ȡID
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
	for (int i = 0; i < m_player.normalfbstatistics_size(); ++i) {
		network::command::NormalFbStatistics* m_norfb = nullptr;
		m_norfb = m_player.mutable_normalfbstatistics(i);
		m_norfb->set_wipetimes(4);
	}
}
