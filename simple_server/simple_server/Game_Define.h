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