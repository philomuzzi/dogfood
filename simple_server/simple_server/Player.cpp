#include "Player.h"
#include "ClientMsgCenter.h"
#include <iostream>

using namespace std;

int Player::global_id_ = 1;

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
	id_ = global_id_;
	global_id_++;
	do_read();
}

// 读取数据之后，解包，然后相应处理
// 消息格式是: 0xFFFF0000|0x0000FFFF,一个int32，其中前16位表示消息id，后面16位表示消息整体长度。后面跟着的是protobuf压缩过的数据
void Player::do_read() {
	auto self(shared_from_this());
	socket_.async_read_some(
		boost::asio::buffer(data_ + already_read_, max_length - already_read_),
		[this, self](boost::system::error_code ec, std::size_t length) {
			if (!ec) {
				already_read_ += length;
				while (already_read_ > HeadLength) {
					int head = *reinterpret_cast<int *>(data_);
					cout << "Head length: " << head << endl;
					int msgid = (head & MSGIDPACKET) >> 16;
					int len = head & MSGLENPACKET;
					if (len > max_length || len < 0) {
						cerr << "头长度错误" << endl;
						already_read_ = 0;
						// 其实应该关闭连接
					} else if (already_read_ >= len) {			
						std::string content(len, '\0');
						copy(data_ + sizeof(head) , data_ + sizeof(head) + len - 1, content.begin());
						copy(data_ + sizeof(head) + len, data_ + already_read_, data_);	// 后面的数据没有清空
						already_read_ -= sizeof(head) + len;

						//将msgid和content放入消息处理函数中
						ClientMsgCenter::getInstance().dispatch(msgid, self, content.data(), len);
					} else {
						break;
					}
				}

				do_read();
			}
		}
	);
}
