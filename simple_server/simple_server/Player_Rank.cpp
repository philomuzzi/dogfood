#include "Player.h"
#include "ClientCommand/server.pb.h"
#include "ClientCommand/game.pb.h"
#include "PlayerManager.h"
#include "GameLogic.h"

using namespace std;
using namespace network::command;

void GamePlayer::checkRanking()
{
//	INFO("刷新排行榜到客户端");

	// 全部在线则走这一步
	Game::Ranking_SC cli_msg;
	getOnlineRankingInfo(cli_msg);

	if (0 != cli_msg.rankinginfolist_size())
	{
		ConstructMsgMacro(network::command::CMSGRanking_SC, cli_msg);
		m_connection->sendCmdMsg(cli_msg__, cli_msg_size__);
	}

	if (m_needCheckWeekRank)
	{
		checkLastWeekRank(cli_msg);
	}

//	sendFriendNextMailTime();
}

void GamePlayer::getOnlineRankingInfo(network::command::Game::Ranking_SC &cli_msg)
{
	RankingInfo *info = nullptr;
	shared_ptr<GamePlayer> fri = nullptr;
	for (int i = 0; i < m_player.friendlist_size(); i++)
	{
		fri = nullptr;
		fri = PlayerManager::getInstance().getPlayerByName(m_player.friendlist(i).c_str());
		if (fri != nullptr)
		{
			info = nullptr;
			info = cli_msg.add_rankinginfolist();
			info->set_name(fri->m_player.name());
			info->set_accid(m_player.friendlist(i));
			info->set_avatar(fri->m_player.avatar());
			info->set_level(fri->m_player.level());
			info->set_exp(fri->m_player.exp());
			info->set_allowenergymail(fri->m_player.allowenergymail());
			if (fri->m_player.has_bestrecord())
			{
				network::command::BestRecord *record = new network::command::BestRecord(fri->m_player.bestrecord());
				info->set_allocated_bestrecord(record);
			}
			if (fri->m_player.has_weekrecord())
			{
				network::command::WeekRecord *record = new network::command::WeekRecord(fri->m_player.weekrecord());
				info->set_allocated_weekrecord(record);
			}

			auto airlist = fri->m_player.airplanelist();
			auto it_air = airlist.begin();
			for (; it_air != airlist.end(); ++it_air)
			{
				auto airplane = info->add_airplanelist();
				airplane->CopyFrom(*it_air);
			}
		}
	}
}

void GamePlayer::checkLastWeekRank(network::command::Game::Ranking_SC &msg)     // 借用排行榜信息来排行成绩
{
	m_needCheckWeekRank = false;

	int rank = 1;
	uint32 record = m_player.lastweekrecord();
	for (int i = 0; i < msg.rankinginfolist_size(); ++i)
	{
		const RankingInfo &info = msg.rankinginfolist(i);
		if (info.lastloginweek() == GameLogic::getLogicInstance()->getWeek())
		{
			if (record < info.lastweekrecord())
				rank += 1;
		}
		else if (info.lastloginweek() == GameLogic::getLogicInstance()->getWeek() - 1)
		{
			if (info.has_weekrecord() && record < info.weekrecord().score())
				rank += 1;
		}

		if (rank > 3)
			break;
	}

//	INFO("本周玩家好友排名为%d", rank);
	switch (rank) {
	case 1:
		m_player.mutable_bestrecord()->set_first(m_player.bestrecord().first() + 1);
		break;
	case 2:
		m_player.mutable_bestrecord()->set_second(m_player.bestrecord().second() + 1);
		break;
	case 3:
		m_player.mutable_bestrecord()->set_third(m_player.bestrecord().third() + 1);
		break;
	}
}

void GamePlayer::checkWeekRanking()
{
	if (m_player.lastloginweek() != GameLogic::getLogicInstance()->getWeek())
	{
		m_player.set_lastweekrecord(0);
		if (m_player.has_weekrecord())
		{
			// 成绩可能是上周登录，本周记录的
			if (m_player.weekrecord().weekindex() <= GameLogic::getLogicInstance()->getWeek() - 1)
			{
				m_player.set_lastweekrecord(m_player.weekrecord().score());
				m_needCheckWeekRank = true;
			}
			m_player.clear_weekrecord();
		}
	}
}