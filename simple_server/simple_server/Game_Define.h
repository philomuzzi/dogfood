#pragma once


#define DailyRewardID 120300000
#define ServerPort 9000
#define IMPOSSIBLE_RETURN (uint32)-1

enum FbType {
	FbType_None = -1,
	FbType_Endless = 1, //无尽模式
	FbType_Nomal = 2, //闯关模式
	FbType_Wipe = 3, //扫荡模式
	FbType_Pvp = 4, //pvp模式
};

enum PlayerItemType {
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

typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef unsigned short uint16;

enum MoneyAction {
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

const char* MoneyActionLog[] = {
	"通过钻石购买金币",
	"通过金钱购买钻石",
	"通过钻石购买体力",
	"减少钻石购买金币",
	"减少钻石增加体力",
	"减少钻石升级飞机",
	"减少钻石购买飞机槽",
	"减少金币升级飞机",
	"减少钻石购买飞机",
	"减少金币购买飞机",
	"减少金币升级角色",
	"减少钻石升级角色",
	"减少金币购买角色",
	"减少钻石购买角色",
	"减少金币升级部件",
	"减少钻石升级部件",
	"减少钻石购买背包空间",
	"出售物品获得金币",
	"减少金币升级技能",
	"减少钻石升级技能",
	"增加金币游戏积分转换",
	"增加钻石游戏关卡积分奖励",
	"GM增加金币",
	"GM减少金币",
	"GM增加钻石",
	"GM减少钻石",
	"完成任务增加钻石",
	"完成任务增加金币",
	"宠物强化减少钻石",
	"宠物强化减少金币",
	"购买宠物蛋减少金币",
	"购买宠物蛋减少钻石",
	"购买宠物涂鸦减少金币",
	"购买宠物涂鸦减少钻石",
	"关卡掉落扫荡券",
	"关卡奖励金币",
	"关卡奖励钻石",
	"签到奖励金币",
	"签到奖励钻石",

	"ERROR"
};