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
	player.set_accid(accid);
	player.set_name(accid);
	player.set_state(0);

	sqlite3_stmt *stmt;
	SerializeMsgMacro(player);
	std::stringstream strm;
	strm << "insert into player (accid, name, account, databinary) values('" << accid
		<< "','" << accid << "','" << accid << "',?);";

	string sql = strm.str();

	if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
		// 这里的1的意思是第一个？
		// 在这里纠结了多久啊~
		sqlite3_bind_blob(stmt, 1, player__, player_size__, NULL);
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
	std::string sql = "select * from player where account = '" + accid + "';";

	if (sqlite3_prepare(db, sql.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		int res = sqlite3_step(statement);

		if (res == SQLITE_ROW)
		{
			// 这里可能是有问题的，因为protobuf序列化后的数据中间可能存在0，所以会导致取回的数据不完整
			char* s0 = (char*)sqlite3_column_blob(statement, static_cast<int>(Player_Field::DATABINARY));
			int len = sqlite3_column_bytes(statement, static_cast<int>(Player_Field::DATABINARY));
			cout << accid << " 读取数据的大小: " << len << endl;
			player.ParsePartialFromArray(s0, len);
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

void DatabaseConnection::updatePlayer(network::command::Player& player) {
	sqlite3_stmt * statement;

	SerializeMsgMacro(player);
	std::stringstream strm;
	strm << "update player set databinary = ? where accid = '" << player.accid() << "';";
	string sql = strm.str();

	cout << player.accid() << " 写入数据的大小: " << player_size__ << endl;

	if (sqlite3_prepare(db, sql.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_blob(statement, 1, player__, player_size__, NULL);
		int res = sqlite3_step(statement);

		sqlite3_finalize(statement);
	}
}