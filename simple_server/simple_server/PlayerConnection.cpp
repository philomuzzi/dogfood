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

	// 需要注意这里的封装是否能够工作
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

// 读取数据之后，解包，然后相应处理
// 消息格式是: 0xFFFF0000|0x0000FFFF,一个int32，其中前16位表示消息id，后面16位表示消息整体长度。后面跟着的是protobuf压缩过的数据
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
						cerr << "头长度错误" << endl;
						already_read_ = 0;
						// 其实应该关闭连接
					}
					else if (already_read_ >= len) {
						std::string content(len, '\0');
						copy(read_buffer_ + sizeof(head), read_buffer_ + sizeof(head) + len, content.begin());
						copy(read_buffer_ + sizeof(head) + len, read_buffer_ + already_read_, read_buffer_); // 后面的数据没有清空
						already_read_ -= sizeof(head) + len;

						//将msgid和content放入消息处理函数中
						ConnectionMsgCenter::getInstance().dispatch(msgid, self, content.data(), len);
					}
					else {
						break;
					}
				}

				do_read();
			}
			if (ec != boost::asio::error::operation_aborted) {
				// 断开连接
			}
		}
	);
}
