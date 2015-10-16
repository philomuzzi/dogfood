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

#define ENDLESSFBID 131000001

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

	Play::StartGame_CS send;
	send.set_accid(rev.data().accid());
	send.set_fbid(ENDLESSFBID);
	send.set_airplaneid(rev.data().currentairplane());
	send.set_pilotid(rev.data().currentpilot());

	ConstructMsgMacro(CMSGResStartGame_CS, send);
	self->sendCmdMsg(send__, send_size__);

	return true;
}

bool ParseStartGame_CS(shared_ptr<Connection> self, const void* msg, const short msglen) {
	cout << __FUNCTION__ << endl;
	Play::StartGame_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	if (rev.result() == Play::StartGame_CS::SUCCESS) {
		std::this_thread::sleep_for(std::chrono::seconds(10));
		Play::EndGame_CS send;
		send.set_accid(rev.accid());
		send.set_fbid(rev.fbid());
		send.set_score(1000);

		ConstructMsgMacro(CMSGResEndGame_CS, send);
		self->sendCmdMsg(send__, send_size__);
	} else {
		cout << "ÓÎÏ·¿ªÊ¼Ê§°Ü£º " << rev.result() << endl;
	}

	return true;
}

bool ParseEndGame_CS(shared_ptr<Connection> self, const void* msg, const short msglen) {
	cout << __FUNCTION__ << endl;
	Play::EndGame_CS rev;
	rev.ParsePartialFromArray(msg, msglen);

	return true;
}

bool ConnectionMsgCenter::registry() {
	registry(CMSGServerInfo_S, PraseServerInfo_S);
	registry(CMSGPlayerInfo_S, ParsePlayerInfo_S);
	registry(CMSGResStartGame_CS, ParseStartGame_CS);
	registry(CMSGResEndGame_CS, ParseEndGame_CS);
	return true;
}
