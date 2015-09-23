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

// 读取数据之后，解包，然后相应处理
// 消息格式是: 0xFFFF0000|0x0000FFFF,一个int32，其中前16位表示消息id，后面16位表示消息整体长度。后面跟着的是protobuf压缩过的数据
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
						std::cerr << "头长度错误" << endl;
						data_ = {};
					} else if (data_.size() >= len) {
						auto begin = data_.begin();						
						std::array<char, max_length> content;
						std::copy(begin + 3, begin + 3 + len, content.begin());
						std::copy(begin + 3 + len, data_.end(), begin);	// 后面的数据没有清空

						//将msgid和content放入消息处理函数中
					}

				}
//				std::cout << std::string(data_.data(), length);
//				do_write(length);

				do_read();
			}
		}
	);
}
