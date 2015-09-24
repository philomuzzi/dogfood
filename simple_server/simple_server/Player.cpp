#include "Player.h"
#include "ClientMsgCenter.h"
#include <iostream>
#include "PlayerManager.h"

using namespace std;

int Player::global_id_ = 1;

void Player::write(std::string data, std::size_t length) {
	auto self(shared_from_this());
	copy(data.begin(), data.end(), write_buffer_.begin());
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(write_buffer_, length),
		[this, self](boost::system::error_code ec, std::size_t len) {
			if (!ec) {
				cout << "write success: " << len << endl;
			}
		}
	);
}

void Player::start() {
	id_ = global_id_;
	global_id_++;
	PlayerManager::getInstance().addUnique(shared_from_this());
	do_read();
}

// ��ȡ����֮�󣬽����Ȼ����Ӧ����
// ��Ϣ��ʽ��: 0xFFFF0000|0x0000FFFF,һ��int32������ǰ16λ��ʾ��Ϣid������16λ��ʾ��Ϣ���峤�ȡ�������ŵ���protobufѹ����������
void Player::do_read() {
	auto self(shared_from_this());
	socket_.async_read_some(
		boost::asio::buffer(read_buffer_ + already_read_, max_length - already_read_),
		[this, self](boost::system::error_code ec, std::size_t length) {
			if (!ec) {
				already_read_ += length;
				while (already_read_ > HeadLength) {
					int head = *reinterpret_cast<int *>(read_buffer_);
					cout << "Head length: " << head << endl;
					int msgid = (head & MSGIDPACKET) >> 16;
					int len = head & MSGLENPACKET;
					if (len > max_length || len < 0) {
						cerr << "ͷ���ȴ���" << endl;
						already_read_ = 0;
						// ��ʵӦ�ùر�����
					} else if (already_read_ >= len) {			
						std::string content(len, '\0');
						copy(read_buffer_ + sizeof(head) , read_buffer_ + sizeof(head) + len, content.begin());
						copy(read_buffer_ + sizeof(head) + len, read_buffer_ + already_read_, read_buffer_);	// ���������û�����
						already_read_ -= sizeof(head) + len;

						//��msgid��content������Ϣ��������
						ClientMsgCenter::getInstance().dispatch(msgid, self, content.data(), len);
					} else {
						break;
					}
				}

				do_read();
			} 
			if (ec != boost::asio::error::operation_aborted)
			{
				PlayerManager::getInstance().delUnique(id_);
			}
		}
	);
}
