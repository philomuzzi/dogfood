#include "ConnectionMsgCenter.h"
#include "message.pb.h"
#include "login_msg.pb.h"
#include "../Utility/utility.h"
#include <iostream>
#include "Connection.h"
#include <play.pb.h>
#include <thread>

using namespace std;
using namespace network::command;

bool PraseServerInfo_S(shared_ptr<Connection> self, const void* msg, const short msglen) {
	cout << __FUNCTION__ << endl;
	ServerInfo_S rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->do_connectToGS(rev);

	return true;
}

bool ParsePlayerInfo_S(shared_ptr<Connection> self, const void* msg, const short msglen) {
	cout << __FUNCTION__ << endl;
	PlayerInfo_S rev;
	rev.ParsePartialFromArray(msg, msglen);

	self->setPlayer(rev.data());
	self->start_game();

	return true;
}

bool ParseStartGame_CS(shared_ptr<Connection> self, const void* msg, const short msglen) {
	cout << __FUNCTION__ << endl;
	Play::StartGame_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	if (rev.result() == Play::StartGame_CS::SUCCESS) {
		cout << "游戏开始: " << rev.fbid() << endl;
		self->end_game();
	} else {
		cout << "游戏开始失败： " << rev.result() << endl;
	}

	return true;
}

bool ParseEndGame_CS(shared_ptr<Connection> self, const void* msg, const short msglen) {
	cout << __FUNCTION__ << endl;
	Play::EndGame_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	if (rev.result() == Play::EndGame_CS::SUCCESS) {
		cout << "游戏结束: " << rev.fbid() << endl;
		self->start_game();
	} else {
		cout << "游戏结束失败: " << rev.result() << endl;
	}

	return true;
}

bool ConnectionMsgCenter::registry() {
	registry(CMSGServerInfo_S, PraseServerInfo_S);
	registry(CMSGPlayerInfo_S, ParsePlayerInfo_S);
	registry(CMSGResStartGame_CS, ParseStartGame_CS);
	registry(CMSGResEndGame_CS, ParseEndGame_CS);
	return true;
}
