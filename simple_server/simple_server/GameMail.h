#pragma once

#include "ClientCommand/mail.pb.h"
#include "ClientCommand/data.pb.h"
#include "ClientCommand/server.pb.h"
#include "Game_Define.h"
#include "Utility/singleton.h"
#include <memory>

using SystemMailMap = std::map<uint64, std::shared_ptr<network::command::MailInfo>> ;

#define DAY_MAIL_TIME 86400     //1��       ��λ����

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
	// �ο�twitter snowflake�㷨
	// 64       63------------22-----------12------------0
	// ����λ   |   41λʱ��  | 10λ������ | 12λ������  |
	uint64 generateUUID();

public:
	SystemMailMap m_systemMail;

private:
	uint32 m_sequence;
};