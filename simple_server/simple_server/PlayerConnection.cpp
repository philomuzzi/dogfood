#include "PlayerConnection.h"
#include <iostream>
#include "ConnectionMsgCenter.h"

using namespace std;

void PlayerConnection::write(std::string data, std::size_t length) {
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

void PlayerConnection::sendCmdMsg(std::string data, int head) {
	auto self(shared_from_this());
	std::array<char, sizeof(int)> arrHead;
	memcpy(arrHead.data(), &head, sizeof(head));

	// ��Ҫע������ķ�װ�Ƿ��ܹ�����
	auto it = copy(arrHead.begin(), arrHead.end(), write_buffer_.begin());
	copy(data.begin(), data.end(), it);

	boost::asio::async_write(
		socket_,
		boost::asio::buffer(write_buffer_, head & PacketMsgLenMask),
		[this, self](boost::system::error_code ec, std::size_t len) {
			if (!ec) {
				cout << "write success: " << len << endl;
			}
		}
	);
}

void PlayerConnection::start() {
	do_read();
}

// ��ȡ����֮�󣬽����Ȼ����Ӧ����
// ��Ϣ��ʽ��: 0xFFFF0000|0x0000FFFF,һ��int32������ǰ16λ��ʾ��Ϣid������16λ��ʾ��Ϣ���峤�ȡ�������ŵ���protobufѹ����������
void PlayerConnection::do_read() {
	auto self(shared_from_this());
	socket_.async_read_some(
		boost::asio::buffer(read_buffer_ + already_read_, Max_DataBufferSize - already_read_),
		[this, self](boost::system::error_code ec, std::size_t length) {
			if (!ec) {
				already_read_ += length;
				while (already_read_ > PacketHeadLen) {
					int head = *reinterpret_cast<int *>(read_buffer_);
					cout << "Head length: " << head << endl;
					int msgid = (head & PacketMsgIdMask) >> 16;
					int len = head & PacketMsgLenMask;
					if (len > Max_DataBufferSize || len < 0) {
						cerr << "ͷ���ȴ���" << endl;
						already_read_ = 0;
						// ��ʵӦ�ùر�����
					}
					else if (already_read_ >= len) {
						std::string content(len, '\0');
						copy(read_buffer_ + sizeof(head), read_buffer_ + sizeof(head) + len, content.begin());
						copy(read_buffer_ + sizeof(head) + len, read_buffer_ + already_read_, read_buffer_); // ���������û�����
						already_read_ -= sizeof(head) + len;

						//��msgid��content������Ϣ��������
						ConnectionMsgCenter::getInstance().dispatch(msgid, self, content.data(), len);
					}
					else {
						break;
					}
				}

				do_read();
			}
			if (ec != boost::asio::error::operation_aborted) {
				// �Ͽ�����
			}
		}
	);
}
