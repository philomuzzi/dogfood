#include "ClientMsgCenter.h"
#include "Player.h"
#include "message.pb.h"
#include "ClientCommand/play.pb.h"
#include <iostream>

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

bool ClientMsgCenter::registry() {
	registry(CMSGResStartGame_CS, ParseStartGame_CS);
	registry(CMSGResContinueGame_CS, ParseContinueGame_CS);
	registry(CMSGResEndGame_CS, ParseEndGame_CS);
	registry(CMSGCheckIn_CS, ParseCheckIn_CS);
	return true;
}
