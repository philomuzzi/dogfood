#pragma once
#include "PlayerConnection.h"
#include "save.pb.h"
#include "play.pb.h"
#include "Utility/utility.h"
#include <memory>
#include "ClientCommand/message.pb.h"
#include "Game_Define.h"
#include "ClientCommand/shop.pb.h"
#include "ClientCommand/game.pb.h"

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
	void calRewardPart(network::command::Play::EndGame_CS& msg, std::string& grade);
	void obtainItem(network::command::Play::EndGame_CS& msg, uint32 fbType);
	void obtainItemFromNormalFb(network::command::Play::EndGame_CS& msg);
	void obtainItemFromWipeFb(network::command::Play::EndGame_CS& msg);
	bool checkEnterNormalFb(network::command::Play::StartGame_CS& msg);
	bool checkEnterEndlessFb(network::command::Play::StartGame_CS& msg);
	bool checkWipeFb(network::command::Play::StartGame_CS& msg);
	void independProbDrop(network::command::Play::EndGame_CS& msg, uint32 id);
	void mutexProbDrop(uint32 dropid, uint32& id, uint32& number);
	void mutextProDrop(network::command::Play::EndGame_CS& msg, uint32 groupid);
	void storageDropOutTable();
	void updateWipeTimes();
	void endGame(network::command::Play::EndGame_CS& msg);

	void checkin(network::command::Play::CheckIn_CS& msg);
	void buyCurrency(network::command::Shop::BuyCurrency_CS& msg);
	void buyGold(network::command::Shop::BuyCurrency_CS& msg);
	void buyDiamond(network::command::Shop::BuyCurrency_CS& msg);
	void buyEnergy(network::command::Shop::BuyCurrency_CS& msg);
	void upgradeObject(network::command::Shop::UpgradeObject_CS& msg);
	void upgradePilot(network::command::Shop::UpgradeObject_CS& msg);
	void upgradeAirplane(network::command::Shop::UpgradeObject_CS& msg);
	void upgradePlanePart(network::command::Shop::UpgradeObject_CS& msg);
	void buyObject(network::command::Shop::BuyObject_CS& msg);
	void buyAirplane(network::command::Shop::BuyObject_CS& msg);
	void buyPilot(network::command::Shop::BuyObject_CS& msg);
	void buyPlanePart(network::command::Shop::BuyObject_CS& msg);
	void buyItem(network::command::Shop::BuyObject_CS& msg);
	void buyPileItem(network::command::Shop::BuyObject_CS& msg);
	void buyBagSpace(network::command::Shop::BuyObject_CS& msg);
	void installPlanePart(network::command::Shop::InstallPlanePart_CS& msg);
	void planePartInstall(network::command::Shop::InstallPlanePart_CS& msg, network::command::PlaneSlot* slot, network::command::PlanePart* part);
	void sellObject(network::command::Shop::SellObject_CS& msg);
	void sellItem(network::command::Shop::SellObject_CS& msg);
	void sellPlanePart(network::command::Shop::SellObject_CS& msg);
	bool checkMoney(network::command::Shop::CurrencyType type, const uint32 num) const;
	void addMoney(network::command::Shop::CurrencyType type, uint32 num, MoneyAction action);
	bool subMoney(network::command::Shop::CurrencyType type, const uint32 num, MoneyAction action);
	
private:
	void initNewPlayer();
	void sendPlayerInfo();
	uint64 generateUUID();
	void sendInitInfo();
	void checkCheckIn();
	void clearCheckIn();
	void setLastLoginTime();
	bool addEnergy(uint32 value);
	bool subEnergy(uint32 value);
	bool subWipeStock(uint32 value);
	bool addWipeStock(uint32 value);
	bool subFbUnlockItem(uint32 value);
	bool addFbUnlockItem(uint32 value);
	network::command::Pilot* getPilot(uint32 id);
	network::command::Airplane* getPlane(uint32 id);
	void addExp(const uint32 value);
	void levelUp(uint32 value);
	void save();	// 需要一个随机时钟来存储它
	void checkRanking();
	void getOnlineRankingInfo(network::command::Game::Ranking_SC& cli_msg);
	void checkLastWeekRank(network::command::Game::Ranking_SC& msg);
	void checkWeekRanking();
	// bag
	uint32 addPetBag(uint32 pet_id);
	bool addPlanePartBag(uint32 part_id, const uint32 add_num);
	bool subPlanePartBag(uint32 thisid);
	bool subPlanePartBag(uint32 id, uint32 num);
	bool addPileItemBag(uint32 item_id, const uint32 add_num);
	bool subPileItemBag(uint32 item_id, const uint32 sub_num);
	bool addNormalItemBag(uint32 item_id, const uint32 add_num = 1);
	bool subNormalItemBag(uint32 item_id, const uint32 sub_num = 1);
	bool addBagSpace(const uint32 space_num);
	void cacuBagSize();
	bool bagHasFull();
	bool addItemToBag(uint32 item_id, uint32 item_num = 1);
	bool subItemBag(uint32 item_id, uint32 item_num = 1);
	void addItem(uint32 id, uint32 num);
	void addGiftToBag(uint32 gift_id);
	void addAirPlane(uint32 id, bool isBuy = false);

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
	bool m_needCheckWeekRank { false };
	static uint32 m_sequence;

	static std::map<uint32, std::vector<uint32>> sm_fbRewardScoreJudge;
	static std::map<uint32, std::vector<uint32>> sm_dropOutVector;
};

