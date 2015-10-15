#include "player.h"

#include "ClientCommand/message.pb.h"
#include "ClientCommand/server.pb.h"
#include "GameMail.h"
#include "GameLogic.h"
#include "PlayerManager.h"

using namespace network::command;
using namespace std;

#define MAIL_ENERGY 5
#define SEND_CLIENT_TIMER 60 * 5

void GamePlayer::loadUserMail(Mail_SS& msg) {
	auto maillist = msg.mutable_maillist();
	auto it_mail = maillist->begin();
	for (; it_mail != maillist->end(); ++it_mail) {
		auto info = m_player.mutable_mailbag()->add_usermail();
		info->CopyFrom(*it_mail);
		//		INFO("��������ʼ�: %ld", info->id());
	}
}

void GamePlayer::sendSystemMail() {
	if (MailManager::getInstance().m_systemMail.empty()) {
		printf("ϵͳ�ʼ�Ϊ��\n");
		return;
	}

	Mail::SystemMail_SC msg;
	for (auto it = MailManager::getInstance().m_systemMail.begin();
	     it != MailManager::getInstance().m_systemMail.end();
	     ++it) {
		auto info = msg.add_maillist();
		info->CopyFrom(*it->second);
	}

//	SerializeMsgMacro(msg);
//	this->sendCmdToMe(CMSGSystemMail_SC, msg__, msg_size__);
}

void GamePlayer::readMail(network::command::Mail::ReadMail_CS& msg) {
	if (msg.stype() == SystemMail) {
		auto it = MailManager::getInstance().m_systemMail.find(msg.id());
		if (it == MailManager::getInstance().m_systemMail.end()) {
			printf("�����ڵ�ϵͳ�ʼ�id��%llu\n", msg.id());

			return ProtocolReturn(msg, Mail::ReadMail_CS::ERROR_ID, CMSGReadMail_CS);
		}

		auto info = it->second;
		if (info == nullptr) {
			printf("δ֪ϵͳ�ʼ�����%llu\n", msg.id());
			return ProtocolReturn(msg, Mail::ReadMail_CS::UNKNOWN, CMSGReadMail_CS);
		}

		// ��ȡ�ʼ���������ȡ��Ʒ
		readMailAttach(info);

		m_player.mutable_mailbag()->add_readsystemmaillist(msg.id());

		sendPlayerInfo();
		return ProtocolReturn(msg, Mail::ReadMail_CS::SUCCESS, CMSGReadMail_CS);
	}
	else if (msg.stype() == UserMail) {
		shared_ptr<MailInfo> info = nullptr;

		auto usermail = m_player.mutable_mailbag()->mutable_usermail();
		auto it_mail = usermail->begin();
		for (; it_mail != usermail->end(); ++it_mail) {
			if (it_mail->id() == msg.id()) {
				// todo, ������������Ȩת���ǲ�������ȷ��
				info = make_shared<MailInfo>(*it_mail);
				break;
			}
		}

		if (info == nullptr) {
			printf("���͵��ʼ�id����, %llu", msg.id());
			return ProtocolReturn(msg, Mail::ReadMail_CS::ERROR_ID, CMSGReadMail_CS);
		}

		readMailAttach(info);
		info->set_status(MailInfo::Read);

		sendPlayerInfo();
		return ProtocolReturn(msg, Mail::ReadMail_CS::SUCCESS, CMSGReadMail_CS);
	}
}

void GamePlayer::readMailAttach(shared_ptr<MailInfo> info) {
	if (info->mailtype() == MailInfo::Attach) {
		for (int i = 0; i < info->attachlist_size(); ++i) {
			addItemToBag(info->attachlist(i).id(), info->attachlist(i).num());
		}
	}
	else if (info->mailtype() == MailInfo::Energy) {
		addEnergy(MAIL_ENERGY);
	}
}

void GamePlayer::createUserMail(network::command::Mail::CreateUserMail_CS& msg) {
	if (msg.recv_accid().empty()) {
		printf("���͵�����ʼ�û�н�����\n");
		return ProtocolReturn(msg, Mail::CreateUserMail_CS::NO_RECV, CMSGCreateUserMail_CS);
	}

	map<string, uint32>::iterator it = m_nextSendClientTime->find(msg.recv_accid());
	if (it != m_nextSendClientTime->end()) {
		if (it->second > GameLogic::m_current_time) {
			printf("�����ʼ�ʱ�仹û�е�����Ҫ��%d���ܷ����ʼ�\n", it->second);
			return ProtocolReturn(msg, Mail::CreateUserMail_CS::WAIT_TIME, CMSGCreateUserMail_CS);
		}
		else {
			printf("����%s��%s���ʼ����ͼ��ʱ��\n", m_player.accid().c_str(), msg.recv_accid().c_str());
			it->second = GameLogic::m_current_time + SEND_CLIENT_TIMER;
		}
	}
	else {
		printf("����%s��%s���ʼ����ͼ��ʱ��\n", m_player.accid().c_str(), msg.recv_accid().c_str());
		m_nextSendClientTime->insert(make_pair(msg.recv_accid(), GameLogic::m_current_time + SEND_CLIENT_TIMER));
	}

	// �ڴ�
	auto info = new MailInfo();
	if (msg.mtype() == Mail::CreateUserMail_CS::ENERGY) {
		info->set_id(generateUUID());
		info->set_stype(UserMail);
		info->set_status(MailInfo::InitMail);
		info->set_mailtype(MailInfo::Energy);
		info->set_time(GameLogic::m_current_time);
		info->set_send_accid(this->m_player.name());
		// �ʼ�����Ϊ�գ�
	}
	else {
		printf("δ֪���ʼ�����, %d\n", msg.mtype());
		delete info;
		return ProtocolReturn(msg, Mail::CreateUserMail_CS::UNKNOWN, CMSGCreateUserMail_CS);
	}

	auto player = PlayerManager::getInstance().getPlayerByName(msg.recv_accid().c_str());
	if (player) {
		// �������
		auto tmp_info = player->m_player.mutable_mailbag()->add_usermail();
		tmp_info->CopyFrom(*info);
		delete info;

		// ֪ͨ������ʼ�?
		player->m_player.set_newmail(true);
		player->sendPlayerInfo();
		player->m_player.set_newmail(false);
	}
	else {
		// ������ߣ���Ҫд�����ݿ�
		WriteMail_SS mailMsg;
		mailMsg.set_send_accid(this->m_player.name());
		mailMsg.set_recv_accid(msg.recv_accid());
		mailMsg.set_allocated_mail(info);

//		SerializeMsgMacro(mailMsg);
//		databaseClient->sendClientCmdToDatabase(CMSGWriteMail_SS, mailMsg__, mailMsg_size__);
//		INFO("д�ʼ������ݿ⣬Recv_id: %s", mailMsg.recv_accid().c_str());
	}
}


void GamePlayer::sendNewSystemMail(const MailInfo& r_info) {
	Mail::SystemMail_SC msg;
	MailInfo* info = msg.add_maillist();
	info->CopyFrom(r_info);

//	SerializeMsgMacro(msg);
//	this->sendCmdToMe(CMSGSystemMail_SC, msg__, msg_size__);

	m_player.set_newmail(true);
	sendPlayerInfo();
	m_player.set_newmail(false);
}


void GamePlayer::checkMail() {
	printf("�������ʼ��Ƿ���ڻ������Ѷ�\n");

	uint64 time;
	uint64 ct = GameLogic::m_current_time;
	uint64 vt;
	auto bag = m_player.mutable_mailbag();
	for (int i = 0; i != bag->usermail_size();) {
		time = bag->usermail(i).time();
		vt = time + 10 * DAY_MAIL_TIME ;
		if (ct > vt || bag->usermail(i).status() == MailInfo::Read) {
			printf("��������ʼ�, %llu", bag->usermail(i).id());
			bag->mutable_usermail()->SwapElements(i, bag->usermail_size() - 1);
			bag->mutable_usermail()->RemoveLast();
		}
		else {
			i++;
		}
	}
}
