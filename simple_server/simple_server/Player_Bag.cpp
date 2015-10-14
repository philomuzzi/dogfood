#include "Player.h"
#include "TableManager.h"

using namespace std;

using namespace network::command;

uint32 GamePlayer::addPetBag(uint32 pet_id)
{
	auto ptr = TableManager::getInstance().getTable("PetProperty");
	uint32 id = ptr->asInt(pet_id, "pet_id");
	if (id != pet_id || id == IMPOSSIBLE_RETURN)
	{
		printf("该宠物编号在宠物表中不存在, %d, %d", pet_id, id);
		return -1;
	}

	PetStatistics *pet = m_player.add_petstatistics();
	pet->set_thisid(generateUUID());
	pet->set_petid(id);
	printf("获得宠物, %d, %d\n", id, pet->thisid());

	return pet->thisid();
}

bool GamePlayer::addPlanePartBag(uint32 part_id, const uint32 add_num)
{
	if (add_num <= 0 || add_num == IMPOSSIBLE_RETURN)
	{
		printf("新增部件数量不对, %d, num: %d", part_id, add_num);
		return false;
	}

	auto ptr = TableManager::getInstance().getTable("PlanePartsProperty");
	uint32 id = ptr->asInt(part_id, "engineid");
	if (id != part_id || id == IMPOSSIBLE_RETURN)
	{
		printf("该飞机部件编号在飞机部件表中不存在, %d, %d", part_id, id);
		return false;
	}

	Bag *bag = m_player.mutable_bag();
	for (uint32 i = 0; i != add_num; ++i)
	{
		PlanePart *part = bag->add_partlist();
		part->set_id(part_id);
		part->set_thisid(generateUUID());
		part->set_exp(0);
		printf("获取飞机部件，%d, %d", part_id, part->thisid());
	}

	bag->set_usedsize(bag->usedsize() + add_num);

	return true;
}

bool GamePlayer::subPlanePartBag(uint32 thisid)
{
	Bag *bag = m_player.mutable_bag();
	for (int i = 0; i != bag->partlist_size(); ++i)
	{
		if (bag->partlist(i).thisid() == thisid)
		{
			bag->mutable_partlist()->SwapElements(i, bag->partlist_size() - 1);
			bag->mutable_partlist()->RemoveLast();
			break;
		}
	}

	bag->set_usedsize(bag->usedsize() - 1);

	return true;
}

bool GamePlayer::subPlanePartBag(uint32 id, uint32 num)
{
	Bag *bag = m_player.mutable_bag();
	uint32 tmp_num = 0;
	for (int i = 0; i != bag->partlist_size() && tmp_num != num; ++i)
	{
		if (bag->partlist(i).id() == id)
		{
			tmp_num += 1;
		}
	}

	if (tmp_num < num)
		return false;

	for (int i = 0; i != bag->partlist_size() && tmp_num > 0; ++i)
	{
		if (bag->partlist(i).id() == id)
		{
			bag->mutable_partlist()->SwapElements(i, bag->partlist_size() - 1);
			bag->mutable_partlist()->RemoveLast();
			tmp_num -= 1;
		}
	}

	bag->set_usedsize(bag->usedsize() - num);

	return true;
}

bool GamePlayer::addPileItemBag(uint32 item_id, const uint32 add_num)
{
	if (add_num <= 0)
	{
		printf("新增堆叠物品数量不对, %d, num: %d", item_id, add_num);
		return false;
	}

	auto ptr = TableManager::getInstance().getTable("PlanePartsProperty");
	uint32 id = ptr->asInt(item_id, "itemid");
	if (id != item_id || id == IMPOSSIBLE_RETURN)
	{
		printf("该堆叠物品在物品表中不存在, %d", item_id);
		return false;
	}

	Bag *bag = m_player.mutable_bag();
	bool bFound = false;
	for (int i = 0; i != bag->pileitemlist_size(); ++i)
	{
		if (bag->pileitemlist(i).id() == item_id)
		{
			bag->mutable_pileitemlist(i)->set_num(bag->pileitemlist(i).num() + add_num);
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		Item *item = bag->add_pileitemlist();
		item->set_id(item_id);
		item->set_num(add_num);
	}

	cacuBagSize();

	return true;
}

bool GamePlayer::subPileItemBag(uint32 item_id, const uint32 sub_num)
{
	if (sub_num <= 0)
	{
		printf("减少堆叠物品数量不对, %d, num: %d", item_id, sub_num);
		return false;
	}

	auto ptr = TableManager::getInstance().getTable("PlayerItem");
	uint32 id = ptr->asInt(item_id, "itemid");
	if (id != item_id || id == IMPOSSIBLE_RETURN)
	{
		printf("该堆叠物品在物品表中不存在, %d", item_id);
		return false;
	}

	Bag *bag = m_player.mutable_bag();
	for (int i = 0; i != bag->pileitemlist_size(); ++i)
	{
		if (bag->pileitemlist(i).id() == item_id)
		{
			if (bag->pileitemlist(i).num() >= sub_num)
			{
				bag->mutable_pileitemlist(i)->set_num(bag->pileitemlist(i).num() - sub_num);
			}
			else
			{
				return false;
				// bag->mutable_pileitemlist(i)->set_num(0);
			}
			break;
		}
	}

	cacuBagSize();

	return true;
}

bool GamePlayer::addNormalItemBag(uint32 item_id, const uint32 add_num)
{
	if (add_num <= 0)
	{
		printf("新增物品数量不对, %d, num: %d", item_id, add_num);
		return false;
	}


	auto ptr = TableManager::getInstance().getTable("PlayerItem");
	uint32 id = ptr->asInt(item_id, "itemid");
	if (id != item_id || id == IMPOSSIBLE_RETURN)
	{
		printf("该物品在物品表中不存在, %d", item_id);
		return false;
	}

	Bag *bag = m_player.mutable_bag();
	bool bFound = false;
	for (int i = 0; i != bag->itemlist_size(); ++i)
	{
		if (bag->itemlist(i).id() == item_id)
		{
			bag->mutable_itemlist(i)->set_num(bag->itemlist(i).num() + add_num);
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		Item *item = bag->add_itemlist();
		item->set_id(item_id);
		item->set_num(add_num);
	}

	bag->set_usedsize(bag->usedsize() + add_num);

	return true;
}

bool GamePlayer::subNormalItemBag(uint32 item_id, const uint32 sub_num)
{
	if (sub_num <= 0)
	{
		printf("减少物品数量不对, %d, num: %d", item_id, sub_num);
		return false;
	}

	auto ptr = TableManager::getInstance().getTable("PlayerItem");
	uint32 id = ptr->asInt(item_id, "itemid");
	if (id != item_id || id == IMPOSSIBLE_RETURN)
	{
		printf("该物品在物品表中不存在, %d", item_id);
		return false;
	}

	Bag *bag = m_player.mutable_bag();
	for (int i = 0; i != bag->itemlist_size(); ++i)
	{
		if (bag->itemlist(i).id() == item_id)
		{
			if (bag->itemlist(i).num() >= sub_num)
			{
				bag->mutable_itemlist(i)->set_num(bag->itemlist(i).num() - sub_num);
			}
			else
			{
				return false;
				// bag->mutable_itemlist(i)->set_num(0);
			}
			break;
		}
	}

	bag->set_usedsize(bag->usedsize() - sub_num);

	return true;
}

bool GamePlayer::addBagSpace(const uint32 space_num)
{
	Bag *bag = m_player.mutable_bag();
	if (bag == NULL)
		return false;

	auto ptr = TableManager::getInstance().getTable("PlayerInit");
	uint32 bag_max_size =ptr->asInt(0, "bag_max_size");

	if (bag->size() + space_num > bag_max_size)
		bag->set_size(bag_max_size);
	else
		bag->set_size(bag->size() + space_num);

	return true;
}

void GamePlayer::cacuBagSize()
{
	const Bag& bag = m_player.bag();
	uint32 size = 0;
	for (int i = 0; i != bag.itemlist_size(); ++i)
	{
		size += bag.itemlist(i).num();
	}

	size += bag.partlist_size();

	// 这是可堆叠的物品，可能还要计算堆叠的堆数量
	for (int i = 0; i != bag.pileitemlist_size(); ++i)
	{
		uint32 num = bag.pileitemlist(i).num();
		if (num)
		{
			auto ptr = TableManager::getInstance().getTable("PlayerItem");
			uint32 pile_num = ptr->asInt(bag.pileitemlist(i).id(), "pile_num");
			if (num % pile_num)
			{
				size += 1;
			}
			size += num / pile_num;
		}
	}

	m_player.mutable_bag()->set_usedsize(size);
}

bool GamePlayer::bagHasFull()
{
	const Bag& bag = m_player.bag();
	return bag.usedsize() >= bag.size();
}

bool GamePlayer::addItemToBag(uint32 item_id, uint32 item_num)
{
	auto ptr = TableManager::getInstance().getTable("PlayerItem");
	uint32 id = ptr->asInt(item_id, "itemid");
	if (id != item_id || id == IMPOSSIBLE_RETURN)
	{
		printf("该物品在物品表中不存在, %d", item_id);
		return false;
	}
	uint32 type = ptr->asInt(item_id, "type");
	switch (static_cast<PlayerItemType>(type)) {
	case PlayerItemType::Item_PlanePart:
		return addPlanePartBag(id, item_num);
	case PlayerItemType::Item_WipeStock:
		addMoney(network::command::Shop::WipeStock, item_num, MoneyAction::AddWipeStockAction_FbDrop);
		return true;
	case PlayerItemType::Item_GiftBag:
		addGiftToBag(item_id);
		break;
	case PlayerItemType::Item_PetInstance:
		addPetBag(id);
		break;
	case PlayerItemType::Item_PlaneInstance:
		if (!getPlane(item_id))
			addAirPlane(item_id);
		break;
	case PlayerItemType::Item_ExpInstance:
		addExp(item_num);
		break;
	case PlayerItemType::Item_GoldInstance:
		addMoney(Shop::Gold, item_num, MoneyAction::AddGoldDailyReward);
		break;
	case PlayerItemType::Item_DiamondInstance:
		addMoney(Shop::Diamond, item_num, MoneyAction::AddDiamondDailyReward);
		break;
	case PlayerItemType::Item_EnergyInstance:
		addEnergy(item_num);
		break;
	default:
	{
		uint32 pile_num = ptr->asInt(item_id, "pile_num");
		if (pile_num == 1)
		{
			return addNormalItemBag(item_id, item_num);
		}
		else
		{
			return addPileItemBag(item_id, item_num);
		}
	}
	}

	return true;
}

bool GamePlayer::subItemBag(uint32 item_id, uint32 item_num)
{
	auto ptr = TableManager::getInstance().getTable("PlayerItem");
	uint32 id = ptr->asInt(item_id, "itemid");
	if (id != item_id || id == IMPOSSIBLE_RETURN)
	{
		printf("该物品在物品表中不存在, %d", item_id);
		return false;
	}

	uint32 type = ptr->asInt(item_id, "type");
	if (type == static_cast<int>(PlayerItemType::Item_PlanePart))
	{
		return subPlanePartBag(id, item_num);
	}

	uint32 pile_num = ptr->asInt(item_id, "pile_num");
	if (pile_num == 1)
	{
		return subNormalItemBag(item_id, item_num);
	}
	else
	{
		return subPileItemBag(item_id, item_num);
	}
}

void GamePlayer::addItem(uint32 id, uint32 num)
{
	this->addItemToBag(id, num);
}

void GamePlayer::addGiftToBag(uint32 gift_id)
{
	auto ptr = TableManager::getInstance().getTable("GiftBag");
	uint32 id = ptr->asInt(gift_id, "id");
	if (id != gift_id || id == IMPOSSIBLE_RETURN)
	{
		printf("该物品在礼包表中不存在, %d", gift_id);
		return;
	}

	uint32 item_id = 0;
	uint32 num = 0;
#define ADD_GIFTBAG_ITEM(key, index) \
    item_id = ptr->asInt(key, "item_id"#index); \
    num = ptr->asInt(key, "quantity"#index); \
    if (item_id != (uint32)-1 && num != (uint32)-1) \
    { \
        printf("gift_id %d, item id: %d, num: %d", id, item_id, num); \
        addItemToBag(item_id, num); \
    } \

	ADD_GIFTBAG_ITEM(id, 1);
	ADD_GIFTBAG_ITEM(id, 2);
	ADD_GIFTBAG_ITEM(id, 3);
	ADD_GIFTBAG_ITEM(id, 4);

#undef ADD_DAILYREWARD_ITEM
}