#include "ClientMsgCenter.h"
#include "Player.h"
#include "message.pb.h"
#include "ClientCommand/play.pb.h"
#include <iostream>
#include "ClientCommand/pet.pb.h"
#include "ClientCommand/mail.pb.h"

using namespace std;
using namespace network::command;


bool ParseStartGame_CS(shared_ptr<GamePlayer> self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;
	Play::StartGame_CS rev;
	rev.ParsePartialFromArray(msg, msglen);
	
	self->startGame(rev);
	return true;
}

bool ParseEndGame_CS(shared_ptr<GamePlayer> self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;
	Play::EndGame_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->endGame(rev);
	return true;
}

bool ParseContinueGame_CS(shared_ptr<GamePlayer> self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;
	Play::ContinueGame_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->continueGame(rev);
	return true;
}

bool ParseCheckIn_CS(shared_ptr<GamePlayer> self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;
	Play::CheckIn_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->checkin(rev);

	return true;
}


bool ParseBuyCurrency(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Shop::BuyCurrency_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->buyCurrency(rev);

	return true;
}

bool ParseUpgradeObject(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Shop::UpgradeObject_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->upgradeObject(rev);

	return true;
}

bool ParseBuyObject(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Shop::BuyObject_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->buyObject(rev);

	return true;
}

bool ParseSellObject(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Shop::SellObject_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->sellObject(rev);

	return true;
}

bool ParseInstallPlanePart(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Shop::InstallPlanePart_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->installPlanePart(rev);

	return true;
}

bool ParseReadMail_CS(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Mail::ReadMail_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

//	self->readMail(rev);

	return true;
}

bool ParseCreateUserMail_CS(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Mail::CreateUserMail_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

//	self->createUserMail(rev);

	return true;
}

bool ParseCreateSystemMail_CS(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	Mail::CreateSystemMail_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

//	MailManager::getInstance().createSystemMail(rev);

	return true;
}

bool ParseFinishQuest_CS(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Game::FinishQuest_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

//	self->finishQuest(rev);

	return true;
}

bool ParsePlayerSetting_CS(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Game::PlayerSetting_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

//	self->setting(rev);

	return true;
}

bool ParseFriendlinessInteract_CS(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Game::FriendlinessInteract_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

//	self->updatePilotFriendliness(rev);

	return true;
}

bool ParseItemForFriendliness_CS(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen) {
	cout << __FUNCTION__ << endl;

	Game::ItemForFriendliness_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

//	self->itemForFriendliness(rev);

	return true;
}

bool ParseStrPet_CS(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen)
{
	cout << __FUNCTION__ << endl;

	Pet::PetStren_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

//	self->strengthenPet(rev);

	return true;
}

bool ParsePetScraw_C(shared_ptr<GamePlayer>self, const void* msg, const uint16 msglen)
{
	cout << __FUNCTION__ << endl;

	Pet::PetScrawl_C rev;
	rev.ParsePartialFromArray(msg, msglen);

//	self->changePetScrawlState(rev);

	return true;
}

bool ClientMsgCenter::registry() {
	registry(CMSGResStartGame_CS, ParseStartGame_CS);
	registry(CMSGResContinueGame_CS, ParseContinueGame_CS);
	registry(CMSGResEndGame_CS, ParseEndGame_CS);	
	registry(CMSGBuyCurrency_CS, ParseBuyCurrency);
	registry(CMSGUpgradeObject_CS, ParseUpgradeObject);
	registry(CMSGBuyObject_CS, ParseBuyObject);
	registry(CMSGInstallPlanePart_CS, ParseInstallPlanePart);
	registry(CMSGSellObject_CS, ParseSellObject);
	registry(CMSGReadMail_CS, ParseReadMail_CS);
	registry(CMSGCreateUserMail_CS, ParseCreateUserMail_CS);
	registry(CMSGCreateSystemMail_CS, ParseCreateSystemMail_CS);
	registry(CMSGFinishQuest_CS, ParseFinishQuest_CS);
	registry(CMSGPlayerSetting_CS, ParsePlayerSetting_CS);
	registry(CMSGFriendlinessInteract_CS, ParseFriendlinessInteract_CS);
	registry(CMSGItemForFriendliness_CS, ParseItemForFriendliness_CS);
	registry(CMSGPetInfo_CS, ParseStrPet_CS);
	registry(CMSGPetScraw_C, ParsePetScraw_C);
	registry(CMSGCheckIn_CS, ParseCheckIn_CS);
	return true;
}
