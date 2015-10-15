#pragma once

#include "ClientCommand/mail.pb.h"
#include "ClientCommand/data.pb.h"
#include "ClientCommand/server.pb.h"
#include "Game_Define.h"
#include "Utility/singleton.h"
#include <memory>

using SystemMailMap = std::map<uint64, std::shared_ptr<network::command::MailInfo>> ;

#define DAY_MAIL_TIME 86400     //1天       单位：秒

class MailManager : public SingletonBase<MailManager> {
	friend class SingletonBase<MailManager>;
	MailManager();
	~MailManager();
	void clear();

public:
	void loadSystemMail(network::command::Mail_SS &msg);
	void createSystemMail(network::command::Mail::CreateSystemMail_CS &msg);
	void checkMail();

private:
	// 参考twitter snowflake算法
	// 64       63------------22-----------12------------0
	// 符号位   |   41位时间  | 10位机器码 | 12位自增码  |
	uint64 generateUUID();

public:
	SystemMailMap m_systemMail;

private:
	uint32 m_sequence;
};