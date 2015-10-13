#include "DatabaseConnection.h"
#include <string>
#include <iostream>
#include <sstream>
#include "Utility/utility.h"
#include "PlayerConnection.h"

using namespace std;
DatabaseConnection::DatabaseConnection()
{
	auto rc = sqlite3_open(R"(F:\Sqlite3\test.db)", &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else {
		fprintf(stdout, "Database opened successfully\n");
	}
}

DatabaseConnection::~DatabaseConnection()
{
	sqlite3_close(db);
}

bool DatabaseConnection::insertNewPlayer(std::string accid) {
	network::command::Player player;
	sqlite3_stmt *stmt;
	SerializeMsgMacro(player);
	std::string seri_player(player__, player_size__);
	std::stringstream strm;
	strm << "insert into player (accid, name, account, databinary) values('" << accid
		<< "','" << accid << "','" << accid << "','" << seri_player << "')";

	string sql = strm.str();
	if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
		int res = sqlite3_step(stmt);
		if (res == SQLITE_DONE) {
			sqlite3_finalize(stmt);
			return true;
		}
	}

	sqlite3_finalize(stmt);
	return false;
}

bool DatabaseConnection::getPlayerFromDB(std::string accid, network::command::Player& player) {
	sqlite3_stmt * statement;
	std::string sql = "select * from player where account = '" + accid + "'";

	if (sqlite3_prepare(db, sql.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		int res = sqlite3_step(statement);

		if (res == SQLITE_ROW)
		{
			// 这里可能是有问题的，因为protobuf序列化后的数据中间可能存在0，所以会导致取回的数据不完整
			string s0 = (char*)sqlite3_column_text(statement, DATABINARY);
			player.ParsePartialFromString(s0);
			sqlite3_finalize(statement);
			return true;
		}
	}

	sqlite3_finalize(statement);
	return false;
}

bool DatabaseConnection::getPlayerByAccount(std::string accid, network::command::Player& player) {
	if (getPlayerFromDB(accid, player)) {
		return true;
	} else {
		if (insertNewPlayer(accid)) {
			getPlayerFromDB(accid, player);
			return true;
		}
	}

	cout << "error while get player from db" << endl;
	return false;
}