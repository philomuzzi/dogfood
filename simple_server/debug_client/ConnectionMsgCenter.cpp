#include "ConnectionMsgCenter.h"
#include "message.pb.h"
#include "login_msg.pb.h"
#include "../Utility/utility.h"
#include <iostream>

using namespace std;
using namespace network::command;

/*
bool ParseTest_C(shared_ptr<PlayerConnection> self, const void* msg, const short msglen) {
	string str(static_cast<const char*>(msg));
	cout << "msg len: " << msglen << endl;
	cout << "msg content: " << str << endl;
	self->write(str, msglen);

	cout << "player size: " << sizeof(PlayerConnection) << endl;
	return true;
}

bool ParseLogic_C(shared_ptr<PlayerConnection> self, const void* msg, const short msglen) {
	cout << __FUNCTION__ << endl;
	LoginGame_C rev;
	rev.ParsePartialFromArray(msg, msglen);

	// 解决帐号建立判断，分配网管服务器之类的问题
	// 根据account找到玩家角色数据库信息

	ServerInfo_S send;
	send.set_accid(rev.account());
	send.set_loginid(1);
	send.set_ip("127.0.0.1");
	send.set_port(ServerPort);
	ConstructMsgMacro(CMSGServerInfo_S, send);
	self->sendCmdMsg(send__, send_size__);
	cout << "期待服务器登录成功" << endl;

	return true;
}

bool ParseEnterGame_CSS(shared_ptr<PlayerConnection> self, const void* msg, const short msglen) {
	cout << __FUNCTION__ << endl;
	EnterGame_CSS rev;
	rev.ParsePartialFromArray(msg, msglen);

	auto player = PlayerManager::getInstance().getPlayerByName(rev.accid());
	if (player) {
		PlayerManager::getInstance().removePlayer(player);
	}

	player = make_shared<GamePlayer>(self, rev.accid());
	if (player) {
		PlayerManager::getInstance().addPlayer(player);
	}

	self->setName(rev.accid());

	auto proto_player = DatabaseCache::getInstance().getPlayer(rev.accid());
	player->online(proto_player);

	return true;
}
*/

bool ConnectionMsgCenter::registry() {
	/*
	registry(10000, ParseTest_C);
	registry(CMSGLoginGame_C, ParseLogic_C);
	registry(CMSGEnterGame_CSS, ParseEnterGame_CSS);
	*/
	return true;
}
