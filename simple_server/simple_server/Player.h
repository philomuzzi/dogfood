#pragma once
#include "PlayerConnection.h"
#include "save.pb.h"
#include "play.pb.h"
#include "Utility/utility.h"
#include <memory>
#include "ClientCommand/message.pb.h"
#include "Game_Define.h"

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

	void checkin(network::command::Play::CheckIn_CS& msg);

private:
	void initNewPlayer();
	void sendPlayerInfo();
	uint64 generateUUID();
	void sendInitInfo();
	void checkCheckIn();
	void clearCheckIn();
	void setLastLoginTime();
	
	// bag
	uint32 addPetBag(uint32 pet_id);
	bool addPlanePartBag(uint32 part_id, const uint32 add_num);
	bool subPlanePartBag(uint32 thisid);
	bool subPlanePartBag(uint32 id, uint32 num);
	bool addPileItemBag(uint32 item_id, const uint32 add_num);
	bool subPileItemBag(uint32 item_id, const uint32 sub_num);
	bool addNormalItemBag(uint32 item_id, const uint32 add_num);
	bool subNormalItemBag(uint32 item_id, const uint32 sub_num);
	bool addBagSpace(const uint32 space_num);
	void cacuBagSize();
	bool bagHasFull();
	bool addItemToBag(uint32 item_id, uint32 item_num = 1);
	bool subItemBag(uint32 item_id, uint32 item_num = 1);
	void addItem(uint32 id, uint32 num);
	void addGiftToBag(uint32 gift_id);

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
	static uint32 m_sequence;
};

