#include "ClientMsgCenter.h"
#include "Player.h"
#include "message.pb.h"
#include "Utility/utility.h"
#include <iostream>

using namespace std;
using namespace network::command;


bool ParseStartGame_CS(shared_ptr<GamePlayer> self, const void* msg, const short msglen) {
	return true;
}

bool ParseEndGame_CS(shared_ptr<GamePlayer> self, const void* msg, const short msglen) {
	return true;
}

bool ParseContinueGame_CS(shared_ptr<GamePlayer> self, const void* msg, const short msglen) {
	return true;
}

bool ClientMsgCenter::registry() {
	registry(CMSGResStartGame_CS, ParseStartGame_CS);
	registry(CMSGResContinueGame_CS, ParseContinueGame_CS);
	registry(CMSGResEndGame_CS, ParseEndGame_CS);
	return true;
}
