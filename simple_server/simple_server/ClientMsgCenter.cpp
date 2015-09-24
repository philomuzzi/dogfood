#include "ClientMsgCenter.h"
#include "PlayerConnection.h"
#include "message.pb.h"
#include "login_msg.pb.h"
#include "Utility/utility.h"
#include <iostream>

using namespace std;
using namespace network::command;

bool ParseTest_C(shared_ptr<PlayerConnection> player, const void *msg, const short msglen) {
	std::string str(static_cast<const char*>(msg));
	cout << "msg len: " << msglen << endl;
	cout << "msg content: " << str << endl;
	player->write(str, msglen);

	cout << "player size: " << sizeof(PlayerConnection) << endl;
	return true;
}

bool ParseLogic_C(shared_ptr<PlayerConnection> player, const void *msg, const short msglen) {
	LoginGame_C rev;
	rev.ParsePartialFromArray(msg, msglen);

	// 解决帐号建立判断，分配网管服务器之类的问题

	ServerInfo_S send;
	send.set_accid("test");
	send.set_loginid(1);
	send.set_ip("127.0.0.1");
	send.set_port(8001);
	ConstructMsgMacro(CMSGServerInfo_S, send);
	player->sendCmdMsg(send__, send_size__);
	cout << "期待服务器登录成功了" << endl;

	return true;
}

bool ClientMsgCenter::registry() {
	registry(10000, ParseTest_C);
	registry(CMSGLoginGame_C, ParseLogic_C);
	return true;
}
