#include "Player.h"
#include "TableManager.h"
#include "GameLogic.h"

using namespace network::command;

void GamePlayer::startGame(network::command::Play::StartGame_CS& msg) {
	auto wls_ptr = TableManager::getInstance().getTable("WayLevelStage");
	int fb_id = wls_ptr->asInt(msg.fbid(), "id");
	if (msg.fbid() != fb_id)
	{
		printf("�ͻ��˴����ĸ���ID��%u�Ƿ���", msg.fbid());
		return ProtocolReturn(msg, Play::StartGame_CS::PARAMETER, network::command::CMSGResStartGame_CS);
	}
	if (m_player_status.playing() == true)
	{
		printf("������Ϸ��, %s", m_player.name().c_str());
		return ProtocolReturn(msg, Play::StartGame_CS::PLAYING, network::command::CMSGResStartGame_CS);
	}

	//if (bagHasFull())
	//{
	//	MDEBUG("�������������ܿ�ʼ��Ϸ");
	//	return ProtocolReturn(msg, Play::StartGame_CS::NOSPACE, network::command::CMSGResStartGame_CS);
	//}

	//Pilot *pilot = getPilot(msg.pilotid());
	//Airplane *air = getPlane(msg.airplaneid());

	//if (pilot == nullptr || air == nullptr)
	//{
	//	printf("����ؿ�:%uʧ�ܣ���ɫ�ͷɻ������ڣ�", msg.fbid());
	//	return ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, network::command::CMSGResStartGame_CS);
	//}

	int fb_type = wls_ptr->asInt(msg.fbid(), "type");

	////��⸱���Ľ�������
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
	//	MERROR("�Ƿ��ĸ�������:%u", fb_type);
	//	return ProtocolReturn(msg, Play::StartGame_CS::UNKNOWN, network::command::CMSGResStartGame_CS);
	//}

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
	if (m_player_status.playing() == false)
	{
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
	if ((FbType_Nomal == type) && (Play::EndGame_CS::FAIL == msg.result()))
	{
		printf("====>��Ҵ���:%u--ʧ��======", msg.fbid());
		m_player_status.set_playing(false);
		msg.set_fbtype(type);
		return ProtocolReturn(msg, Play::EndGame_CS::FAIL, network::command::CMSGResEndGame_CS);
	}

	//-----------------�����Ľ���------------------
	uint32 tmp_score = msg.score();
	int m_add_score = 0;
	int m_add_exp = 0;

	if (FbType_Endless == type)
	{
//		obtainItem(msg, FbType_Endless);
		// ��������˼�¼�������
		// ͳ���޾�ģʽ
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
		// ������Ҫ��ȡ�����������ñ��
		if (msg.continuecount() >= 10)
		{
			msg.set_result(Play::ContinueGame_CS::ERR);
			msg.set_type(Play::ContinueGame_CS::BUY);

			ConstructMsgMacro(network::command::CMSGResContinueGame_CS, msg);
			m_connection->sendCmdMsg(msg__, msg_size__);

			sendPlayerInfo();
			return;
		}

		// ��ȡ���
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
