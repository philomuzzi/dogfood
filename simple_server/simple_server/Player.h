#pragma once
#include "PlayerConnection.h"
#include <save.pb.h>

#include <memory>

class GamePlayer : public std::enable_shared_from_this<GamePlayer> {
public:
	GamePlayer(std::shared_ptr<PlayerConnection> conn) : connection_(conn) {};
	~GamePlayer();

	std::string getAccid() const {
		return player_.accid();
	}

private:
	std::shared_ptr<PlayerConnection> connection_;

	network::command::Player player_;
};

