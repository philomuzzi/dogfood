#pragma once


#define DailyRewardID 120300000
#define ServerPort 9000
#define IMPOSSIBLE_RETURN (uint32)-1

enum FbType {
	FbType_None = -1,
	FbType_Endless = 1, //�޾�ģʽ
	FbType_Nomal = 2, //����ģʽ
	FbType_Wipe = 3, //ɨ��ģʽ
	FbType_Pvp = 4, //pvpģʽ
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
	"ͨ����ʯ������",
	"ͨ����Ǯ������ʯ",
	"ͨ����ʯ��������",
	"������ʯ������",
	"������ʯ��������",
	"������ʯ�����ɻ�",
	"������ʯ����ɻ���",
	"���ٽ�������ɻ�",
	"������ʯ����ɻ�",
	"���ٽ�ҹ���ɻ�",
	"���ٽ��������ɫ",
	"������ʯ������ɫ",
	"���ٽ�ҹ����ɫ",
	"������ʯ�����ɫ",
	"���ٽ����������",
	"������ʯ��������",
	"������ʯ���򱳰��ռ�",
	"������Ʒ��ý��",
	"���ٽ����������",
	"������ʯ��������",
	"���ӽ����Ϸ����ת��",
	"������ʯ��Ϸ�ؿ����ֽ���",
	"GM���ӽ��",
	"GM���ٽ��",
	"GM������ʯ",
	"GM������ʯ",
	"�������������ʯ",
	"����������ӽ��",
	"����ǿ��������ʯ",
	"����ǿ�����ٽ��",
	"������ﵰ���ٽ��",
	"������ﵰ������ʯ",
	"�������Ϳѻ���ٽ��",
	"�������Ϳѻ������ʯ",
	"�ؿ�����ɨ��ȯ",
	"�ؿ��������",
	"�ؿ�������ʯ",
	"ǩ���������",
	"ǩ��������ʯ",

	"ERROR"
};