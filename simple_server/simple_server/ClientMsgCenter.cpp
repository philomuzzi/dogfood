#include "ClientMsgCenter.h"
#include "Player.h"
#include <iostream>

using namespace std;

bool ParseLogic_C(shared_ptr<Player> player, const void *msg, const short msglen) {
	std::string str(static_cast<const char*>(msg));
	cout << "msg len: " << msglen << endl;
	cout << "msg content: " << str << endl;
	player->write(str, msglen);

//	for (auto c : str)
//		cout << c << endl;

	cout << "player size: " << sizeof(Player) << endl;
	return true;
}

bool ClientMsgCenter::registry() {
	registry(1, ParseLogic_C);
	return true;
}
