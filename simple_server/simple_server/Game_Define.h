#pragma once


#define DailyRewardID 120300000
#define ServerPort 9000
#define IMPOSSIBLE_RETURN (uint32)-1

enum class FbType {
	FbType_None = -1,
	FbType_Endless = 1, //�޾�ģʽ
	FbType_Nomal = 2, //����ģʽ
	FbType_Wipe = 3, //ɨ��ģʽ
	FbType_Pvp = 4, //pvpģʽ
};

enum class PlayerItemType {
	Item_Exp = 1,
	Item_Upgrade,
	Item_Pet,
	Item_Interact,
	Item_PlanePart,
	Item_WipeStock = 7,
	Item_GiftBag = 8,
	Item_PetInstance = 94,
	Item_PlaneInstance,
	Item_ExpInstance,
	Item_GoldInstance,
	Item_DiamondInstance,
	Item_EnergyInstance
};

using uint32 = unsigned int;
using uint64 = unsigned long long;
using uint16 = unsigned short;

enum class MoneyAction {
	AddGoldUseDiamond,
	AddDiamondUseMoney,
	AddEnergyUseDiamond,
	SubDiamondForGold,
	SubDiamondForEnergy,
	SubDiamondForAirplaneUpgrade,
	SubDiamondForAirplaneSlot,
	SubGoldForAirplaneUpgrade,
	SubDiamondForAirplaneBuy,
	SubGoldForAirplaneBuy,
	SubGoldForPilotUpgrade,
	SubDiamondForPilotUpgrade,
	SubGoldForPilotBuy,
	SubDiamondForPilotBuy,
	SubGoldForPartsUpgrade,
	SubDiamondForPartsUpgrade,
	SubDiamondForBagSpace,
	AddGoldSellObject,
	SubGoldForSkill,
	SubDiamondForSkill,
	AddGoldScoreReward,
	AddDiamondScoreReward,
	AddMoneyAction_GM,
	SubMoneyAction_GM,
	AddDiamondAction_GM,
	SubDiamondAction_GM,
	AddDiamondTask,
	AddGoldTask,
	SubDiamondAction_PetStr,
	SubGoldAction_PetStr,
	SubGoldAction_BuyPetEgg,
	SubDiamondAction_BuyPetEgg,
	SubGoldAction_BuyPetScrawl,
	SubDiamondAction_BuyPetScrawl,
	AddWipeStockAction_FbDrop,
	AddGoldAction_FbDrop,
	AddDiamondAction_FbDrop,
	AddGoldDailyReward,
	AddDiamondDailyReward,

	MoneyAction_Max = 255
};