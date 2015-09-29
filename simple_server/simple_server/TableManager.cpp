#include "TableManager.h"
#include "CsvReader.h"

using namespace std;

TableManager::TableManager()
{
}

TableManager::~TableManager()
{
}

std::shared_ptr<CsvReader> TableManager::getManager(const std::string csv_string) {
	if (m_tableMap.find(csv_string) == m_tableMap.end())
		return nullptr;

	return m_tableMap.at(csv_string);
}

void TableManager::initAll() {
	init("BuyDiamond");
	init("BuyEnergy");
	init("ExchangeGold");
	init("PlayerInit");
	init("LevelUpExp");
	init("PlaneProperty");
	init("WayLevelStage");
	init("RewardTable");
	init("BuyPlane");
	init("PilotProperty");
	init("BuyPilot");
	init("PlanePartsProperty");
	init("PlayerItem");
	init("BuyBagSpace");
	init("PilotSkillProperty");
	init("TaskInfo");
	init("PetProperty");
	init("InteractDialog");
	init("DropOut");
	init("DailyReward");
	init("GiftBag");
}

void TableManager::init(const string filename) {
	shared_ptr<CsvReader> buy_diamond_ptr = make_shared<CsvReader>(filename);
	m_tableMap.insert({ filename, buy_diamond_ptr->shared_from_this() });
}