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
		//		INFO("插入玩家邮件: %ld", info->id());
	}
}

void GamePlayer::sendSystemMail() {
	if (MailManager::getInstance().m_systemMail.empty()) {
		printf("系统邮件为空\n");
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
			printf("不存在的系统邮件id，%llu\n", msg.id());

			return ProtocolReturn(msg, Mail::ReadMail_CS::ERROR_ID, CMSGReadMail_CS);
		}

		auto info = it->second;
		if (info == nullptr) {
			printf("未知系统邮件错误，%llu\n", msg.id());
			return ProtocolReturn(msg, Mail::ReadMail_CS::UNKNOWN, CMSGReadMail_CS);
		}

		// 读取邮件奖励，获取物品
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
				// todo, 检查这里的所有权转换是不是这正确的
				info = make_shared<MailInfo>(*it_mail);
				break;
			}
		}

		if (info == nullptr) {
			printf("发送的邮件id有误, %llu", msg.id());
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
		printf("发送的玩家邮件没有接收人\n");
		return ProtocolReturn(msg, Mail::CreateUserMail_CS::NO_RECV, CMSGCreateUserMail_CS);
	}

	map<string, uint32>::iterator it = m_nextSendClientTime->find(msg.recv_accid());
	if (it != m_nextSendClientTime->end()) {
		if (it->second > GameLogic::m_current_time) {
			printf("发送邮件时间还没有到，需要到%d才能发送邮件\n", it->second);
			return ProtocolReturn(msg, Mail::CreateUserMail_CS::WAIT_TIME, CMSGCreateUserMail_CS);
		}
		else {
			printf("更新%s到%s的邮件发送间隔时间\n", m_player.accid().c_str(), msg.recv_accid().c_str());
			it->second = GameLogic::m_current_time + SEND_CLIENT_TIMER;
		}
	}
	else {
		printf("插入%s到%s的邮件发送间隔时间\n", m_player.accid().c_str(), msg.recv_accid().c_str());
		m_nextSendClientTime->insert(make_pair(msg.recv_accid(), GameLogic::m_current_time + SEND_CLIENT_TIMER));
	}

	// 内存
	auto info = new MailInfo();
	if (msg.mtype() == Mail::CreateUserMail_CS::ENERGY) {
		info->set_id(generateUUID());
		info->set_stype(UserMail);
		info->set_status(MailInfo::InitMail);
		info->set_mailtype(MailInfo::Energy);
		info->set_time(GameLogic::m_current_time);
		info->set_send_accid(this->m_player.name());
		// 邮件内容为空？
	}
	else {
		printf("未知的邮件类型, %d\n", msg.mtype());
		delete info;
		return ProtocolReturn(msg, Mail::CreateUserMail_CS::UNKNOWN, CMSGCreateUserMail_CS);
	}

	auto player = PlayerManager::getInstance().getPlayerByName(msg.recv_accid().c_str());
	if (player) {
		// 玩家在线
		auto tmp_info = player->m_player.mutable_mailbag()->add_usermail();
		tmp_info->CopyFrom(*info);
		delete info;

		// 通知玩家新邮件?
		player->m_player.set_newmail(true);
		player->sendPlayerInfo();
		player->m_player.set_newmail(false);
	}
	else {
		// 玩家离线，需要写入数据库
		WriteMail_SS mailMsg;
		mailMsg.set_send_accid(this->m_player.name());
		mailMsg.set_recv_accid(msg.recv_accid());
		mailMsg.set_allocated_mail(info);

//		SerializeMsgMacro(mailMsg);
//		databaseClient->sendClientCmdToDatabase(CMSGWriteMail_SS, mailMsg__, mailMsg_size__);
//		INFO("写邮件进数据库，Recv_id: %s", mailMsg.recv_accid().c_str());
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
	printf("检查玩家邮件是否过期或者是已读\n");

	uint64 time;
	uint64 ct = GameLogic::m_current_time;
	uint64 vt;
	auto bag = m_player.mutable_mailbag();
	for (int i = 0; i != bag->usermail_size();) {
		time = bag->usermail(i).time();
		vt = time + 10 * DAY_MAIL_TIME ;
		if (ct > vt || bag->usermail(i).status() == MailInfo::Read) {
			printf("清理玩家邮件, %llu", bag->usermail(i).id());
			bag->mutable_usermail()->SwapElements(i, bag->usermail_size() - 1);
			bag->mutable_usermail()->RemoveLast();
		}
		else {
			i++;
		}
	}
}
