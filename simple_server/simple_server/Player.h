#pragma once
#include "PlayerConnection.h"
#include "save.pb.h"
#include "play.pb.h"
#include "Utility/utility.h"
#include <memory>
#include "ClientCommand/message.pb.h"

enum FbType {
	FbType_None = -1,
	FbType_Endless = 1, //�޾�ģʽ
	FbType_Nomal = 2, //����ģʽ
	FbType_Wipe = 3, //ɨ��ģʽ
	FbType_Pvp = 4, //pvpģʽ
};

typedef unsigned int uint32;

class GamePlayer : public std::enable_shared_from_this<GamePlayer> {
public:
	GamePlayer(std::shared_ptr<PlayerConnection> conn, const std::string accid) : m_connection(conn) {
		m_player.set_accid(accid);
	}
	~GamePlayer();

	std::string getAccid() const {
		return m_player.accid();
	}

	void online(const network::command::Player& pb_player);

	// game play
	void startGame(network::command::Play::StartGame_CS& msg);
	void continueGame(network::command::Play::ContinueGame_CS& msg);
	void endGame(network::command::Play::EndGame_CS& msg);

private:
	void initNewPlayer();
	void sendPlayerInfo();

	template <typename T1, typename T2> void ProtocolReturn(T1 &msg, T2 result, network::command::CMSG messageName)
	{
		msg.set_result(result);
		ConstructMsgMacro(messageName, msg);
		m_connection->sendCmdMsg(msg__, msg_size__);
	}

private:
	std::shared_ptr<PlayerConnection> m_connection;

	network::command::Player m_player;
	network::command::PlayerStatus m_player_status;
};

