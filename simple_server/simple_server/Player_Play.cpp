#include "Player.h"
#include "TableManager.h"
#include "GameLogic.h"

using namespace network::command;

void GamePlayer::startGame(network::command::Play::StartGame_CS& msg) {
	auto wls_ptr = TableManager::getInstance().getTable("WayLevelStage");
	int fb_id = wls_ptr->asInt(msg.fbid(), "id");
	if (msg.fbid() != fb_id)
	{
		printf("客户端传来的副本ID：%u非法！", msg.fbid());
		return ProtocolReturn(msg, Play::StartGame_CS::PARAMETER, network::command::CMSGResStartGame_CS);
	}
	if (m_player_status.playing() == true)
	{
		printf("正在游戏中, %s", m_player.name().c_str());
		return ProtocolReturn(msg, Play::StartGame_CS::PLAYING, network::command::CMSGResStartGame_CS);
	}

	//if (bagHasFull())
	//{
	//	MDEBUG("背包已满，不能开始游戏");
	//	return ProtocolReturn(msg, Play::StartGame_CS::NOSPACE, network::command::CMSGResStartGame_CS);
	//}

	//Pilot *pilot = getPilot(msg.pilotid());
	//Airplane *air = getPlane(msg.airplaneid());

	//if (pilot == nullptr || air == nullptr)
	//{
	//	printf("进入关卡:%u失败，角色和飞机不存在！", msg.fbid());
	//	return ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, network::command::CMSGResStartGame_CS);
	//}

	int fb_type = wls_ptr->asInt(msg.fbid(), "type");

	////检测副本的进入条件
	//if (FbType_Endless == fb_type || FbType_Pvp == fb_type)
	//{
	//	if (!checkEnterEndlessFb(msg))
	//		return;
	//}
	//else if (FbType_Wipe == msg.fbtype() && FbType_Nomal == fb_type)
	//{
	//	if (!checkWipeFb(msg))
	//		return;
	//}
	//else if (FbType_Nomal == fb_type)
	//{
	//	if (!checkEnterNormalFb(msg))
	//		return;
	//}
	//else
	//{
	//	MERROR("非法的副本类型:%u", fb_type);
	//	return ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, network::command::CMSGResStartGame_CS);
	//}

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

	return ProtocolReturn(msg, Play::StartGame_CS::SUCCESS, network::command::CMSGResStartGame_CS);
}

void GamePlayer::endGame(network::command::Play::EndGame_CS& msg) {
	if (m_player_status.playing() == false)
	{
		printf("没有正在进行的游戏, %s", m_player.name().c_str());
		return ProtocolReturn(msg, Play::EndGame_CS::NOPLAYING, network::command::CMSGResEndGame_CS);
	}
	//此处有个判断是否作弊的接口
	auto wls_ptr = TableManager::getInstance().getTable("WayLevelStage");
	int tmp_id = wls_ptr->asInt(msg.fbid(), "id");
	if (msg.fbid() != tmp_id)//防止客户端传入参数错误
	{
		printf("客户端传入的无限副本唯一标示符错误:%u, %d", msg.fbid(), tmp_id);

		m_player_status.set_playing(false);
		return ProtocolReturn(msg, Play::EndGame_CS::UNKNOWN, network::command::CMSGResEndGame_CS);
	}

	int type = wls_ptr->asInt(msg.fbid(), "type");
	if ((FbType_Nomal == type) && (Play::EndGame_CS::FAIL == msg.result()))
	{
		printf("====>玩家闯关:%u--失败======", msg.fbid());
		m_player_status.set_playing(false);
		msg.set_fbtype(type);
		return ProtocolReturn(msg, Play::EndGame_CS::FAIL, network::command::CMSGResEndGame_CS);
	}

	//-----------------副本的结算------------------
	uint32 tmp_score = msg.score();
	int m_add_score = 0;
	int m_add_exp = 0;

	if (FbType_Endless == type)
	{
//		obtainItem(msg, FbType_Endless);
		// 如果更新了记录，则更新
		// 统计无尽模式
		if (tmp_score > m_player.weekrecord().score())
		{
//			m_player.mutable_weekrecord()->set_weekindex(GameLogic::m_currentWeek);
			m_player.mutable_weekrecord()->set_score(tmp_score);
//			m_player.mutable_weekrecord()->set_time(GameLogic::m_current_time.sec());

			if (tmp_score > m_player.bestrecord().record().score())
			{
				WeekRecord *record = new WeekRecord(m_player.weekrecord());
				m_player.mutable_bestrecord()->set_allocated_record(record);
			}
		}

		if (tmp_score > m_player.totalstatistics().endlesstopscore())
			m_player.mutable_totalstatistics()->set_endlesstopscore(tmp_score);
		m_player.mutable_totalstatistics()->set_endlesstimes(m_player.totalstatistics().endlesstimes() + 1);
		m_player.mutable_dailystatistics()->set_endlesstimes(m_player.dailystatistics().endlesstimes() + 1);
	}
	else if (FbType_Pvp == type) {
//		obtainItem(msg, FbType_Pvp);
	}
	else if (FbType_Nomal == type/*&&Play::EndGame_CS::SUCCESS==msg.result()*/)
	{
//		obtainItemFromNormalFb(msg);
	}
	else
	{
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
//	checkRanking();
	return ProtocolReturn(msg, Play::EndGame_CS::SUCCESS, network::command::CMSGResEndGame_CS);
}

void GamePlayer::continueGame(network::command::Play::ContinueGame_CS& msg) {
	if (msg.type() == Play::ContinueGame_CS::FREE)
	{
		msg.set_result(Play::ContinueGame_CS::SUCCESS);
		msg.set_type(Play::ContinueGame_CS::FREE);
		ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
		m_connection->sendCmdMsg(msg__, msg_size__);

		sendPlayerInfo();
	}
	else if (msg.type() == Play::ContinueGame_CS::BUY)
	{
		// 这里需要读取最大的买活的配置表格
		if (msg.continuecount() >= 10)
		{
			msg.set_result(Play::ContinueGame_CS::ERR);
			msg.set_type(Play::ContinueGame_CS::BUY);

			ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
			m_connection->sendCmdMsg(msg__, msg_size__);

			sendPlayerInfo();
			return;
		}

		// 读取表格
		uint32 price = 10;

		if (price <= m_player.diamond())
		{
			m_player.set_diamond(m_player.diamond() - price);

			msg.set_result(Play::ContinueGame_CS::SUCCESS);
			msg.set_type(Play::ContinueGame_CS::BUY);
			ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
			m_connection->sendCmdMsg(msg__, msg_size__);

			sendPlayerInfo();
		}
		else
		{
			msg.set_result(Play::ContinueGame_CS::FAIL);
			msg.set_type(Play::ContinueGame_CS::BUY);

			ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
			m_connection->sendCmdMsg(msg__, msg_size__);

			sendPlayerInfo();
		}
	}
}
