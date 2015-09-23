#include "Player.h"
#include <iostream>

using namespace std;

void Player::do_write(std::size_t length) {
	auto self(shared_from_this());
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(data_, length),
		[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec) {
				do_read();
			}
		}
	);
}

void Player::start() {
	do_read();
}

// ��ȡ����֮�󣬽����Ȼ����Ӧ����
// ��Ϣ��ʽ��: 0xFFFF0000|0x0000FFFF,һ��int32������ǰ16λ��ʾ��Ϣid������16λ��ʾ��Ϣ���峤�ȡ�������ŵ���protobufѹ����������
void Player::do_read() {
	auto self(shared_from_this());
	socket_.async_read_some(
		boost::asio::buffer(data_, max_length),
		[this, self](boost::system::error_code ec, std::size_t length) {
			if (!ec) {
				while (data_.size() > HeadLength) {
					const char* _data = data_.data();
					int head = *(int *)_data;
					std::cout << "Head length: " << head;
					int msgid = head >> 16;
					int len = head & MSGLENPACKET;
					if (len > max_length || len < 0) {
						std::cerr << "ͷ���ȴ���" << endl;
						data_ = {};
					} else if (data_.size() >= len) {
						auto begin = data_.begin();						
						std::array<char, max_length> content;
						std::copy(begin + 3, begin + 3 + len, content.begin());
						std::copy(begin + 3 + len, data_.end(), begin);	// ���������û�����

						//��msgid��content������Ϣ��������
					}

				}
//				std::cout << std::string(data_.data(), length);
//				do_write(length);

				do_read();
			}
		}
	);
}
