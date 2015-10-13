#pragma once

#include "sqlite3.h"
#include "Utility/singleton.h"
#include <string>
#include "save.pb.h"

class DatabaseConnection : public SingletonBase<DatabaseConnection>
{
	DatabaseConnection();
	~DatabaseConnection();
	friend SingletonBase<DatabaseConnection>;
	sqlite3* db{nullptr};

	enum Player_Field {
		ACCID = 0,
		CHARID = 1,
		NAME = 2,
		ACCOUNT = 3,
		DATABINARY = 4
	};

	bool insertNewPlayer(std::string accid);
	bool getPlayerFromDB(std::string accid, network::command::Player &player);

public:
	bool getPlayerByAccount(std::string, network::command::Player& player);
	void updatePlayer(network::command::Player& player);
};

