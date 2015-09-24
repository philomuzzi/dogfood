#include "ClientMsgCenter.h"
#include "Player.h"
#include "message.pb.h"
#include "login_msg.pb.h"
#include "Utility/utility.h"
#include <iostream>
#include "PlayerManager.h"

using namespace std;
using namespace network::command;

bool ClientMsgCenter::registry() {
	return true;
}
