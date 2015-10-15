#include "GameMail.h"


#include "ClientCommand/data.pb.h"
#include "GameLogic.h"


using namespace network::command;
using namespace std;

MailManager::MailManager() : m_sequence(0)
{
}

MailManager::~MailManager()
{
	clear();
}

void MailManager::clear()
{
	m_systemMail.clear();
}

void MailManager::loadSystemMail(Mail_SS &msg)
{
	if (msg.systemmailtype() == Mail_SS::InitSystemMail)
		clear();

	auto maillist = msg.mutable_maillist();	
	for (auto it_mail = maillist->begin(); it_mail != maillist->end(); ++it_mail)
	{
		auto info = make_shared<MailInfo>(*it_mail);
		m_systemMail.insert(make_pair(info->id(), info));
		printf("����ϵͳ�ʼ�: %llu\n", info->id());

		if (msg.systemmailtype() == Mail_SS::NewMail)
		{
			// �������ʼ����������	
		}
	}
}

void MailManager::createSystemMail(Mail::CreateSystemMail_CS &msg)
{
}

uint64 MailManager::generateUUID()
{
	uint64 value = 0;
	// ����ټ�һ��epoch
	value = GameLogic::m_current_time;
	value = value << 22;
	// ���ÿ��GS��һ���Լ�Ψһ��ʶ
	value |= (1 & 0x3ff) << 12;
	value |= m_sequence++ & 0xFFF;
	if (m_sequence == 0x1000)
		m_sequence = 0;

	return value;
}

void MailManager::checkMail()
{
//	INFO("���ϵͳ�ʼ��Ƿ����");

	uint64 time = 0;
	uint64 ct = GameLogic::m_current_time;
	uint64 vt(0);
	for (auto it = m_systemMail.begin(); it != m_systemMail.end(); )
	{
		time = it->second->time();
		vt = time + 10 * DAY_MAIL_TIME;
		if (ct > vt)
		{
//			INFO("����ϵͳ�ʼ�, %ld", it->second->id());
			m_systemMail.erase(it++);
		}
		else
		{
			++it;
		}
	}
}
