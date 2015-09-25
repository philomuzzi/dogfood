#pragma once
#include "PlayerConnection.h"
#include <save.pb.h>

#include <memory>

class GamePlayer : public std::enable_shared_from_this<GamePlayer> {
public:
	GamePlayer(std::shared_ptr<PlayerConnection> conn, const std::string accid) : connection_(conn) {
		player_.set_accid(accid);
	}
	~GamePlayer();

	std::string getAccid() const {
		return player_.accid();
	}

	void online();

	// game play
	void startGame();
	void continueGame();
	void endGame();

private:
	std::shared_ptr<PlayerConnection> connection_;

	network::command::Player player_;
};

