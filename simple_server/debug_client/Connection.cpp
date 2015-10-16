#include "Connection.h"
#include <iostream>
#include "ConnectionMsgCenter.h"
#include "../Utility/define.h"
#include <login_msg.pb.h>
#include <message.pb.h>
#include "../Utility/utility.h"
#include <play.pb.h>

using namespace std;
using namespace boost::asio;
using namespace network::command;

#define ENDLESSFBID 131000001

// Connection的生命周期问题

Connection::~Connection() {
	cout << "Destroy Connection" << endl;
}


void Connection::write(string data, size_t length) {
	auto self(shared_from_this());
	copy(data.begin(), data.end(), write_buffer_.begin());
	boost::asio::async_write(
		socket_,
		buffer(write_buffer_, length),
		[this, self](boost::system::error_code ec, size_t len) {
			if (!ec) {
				cout << "write success: " << len << endl;
			}
		}
	);
}

void Connection::sendCmdMsg(const char* data, int head) {
	auto self(shared_from_this());
	array<char, sizeof(int)> arrHead;
	memcpy(arrHead.data(), &head, sizeof(head));

	// 需要注意这里的封装是否能够工作
	auto it = copy(arrHead.begin(), arrHead.end(), write_buffer_.begin());
	//	copy(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + (head & PacketMsgLenMask), it);
	copy(data, data + (head & PacketMsgLenMask), it);

	boost::asio::async_write(
		socket_,
		buffer(write_buffer_, (head & PacketMsgLenMask) + sizeof(head)),
		[this, self](boost::system::error_code ec, size_t len) {
			if (!ec) {
				cout << "write success: " << len << endl;
			}
		}
	);
}

void Connection::do_connectToGS(ServerInfo_S rev) {
	auto self(shared_from_this());
	if (socket_.is_open())
		socket_.close();
	socket_.async_connect(ip::tcp::endpoint(ip::address::from_string(rev.ip()), rev.port()),
	                      [self, rev](boost::system::error_code ec) {
		                      if (!ec) self->do_enterGame(rev);
		                      else self->stop();
	                      });
}

void Connection::start_game() {
	if (m_startLoop)
		return;

	m_startLoop = true;

	Play::StartGame_CS send;
	send.set_accid(m_player.accid());
	send.set_fbid(ENDLESSFBID);
	send.set_airplaneid(m_player.currentairplane());
	send.set_pilotid(m_player.currentpilot());

	ConstructMsgMacro(CMSGResStartGame_CS, send);
	sendCmdMsg(send__, send_size__);
}

void Connection::end_game() {
	m_startLoop = false;

	Play::EndGame_CS send;
	send.set_accid(m_player.accid());
	send.set_fbid(ENDLESSFBID);
	send.set_score(1000);

	ConstructMsgMacro(CMSGResEndGame_CS, send);
	sendCmdMsg(send__, send_size__);
}

void Connection::do_enterGame(ServerInfo_S rev) {
	EnterGame_CSS send;
	send.set_loginid(rev.loginid());
	send.set_accid(rev.accid());
	send.set_version(0);
	ConstructMsgMacro(network::command::CMSGEnterGame_CSS, send);
	sendCmdMsg(send__, send_size__);
//	cout << "登录GS服务器" << endl;

	do_read();
}

void Connection::stop() {
	socket_.close();
	cout << "close socket" << endl;
}

void Connection::do_connectToPL() {
	LoginGame_C send;
	send.set_account(m_name);
	ConstructMsgMacro(network::command::CMSGLoginGame_C, send);
	sendCmdMsg(send__, send_size__);
//	cout << "登录PL服务器" << endl;

	do_read();
}

void Connection::do_connection(ip::tcp::endpoint ep) {
	auto self(shared_from_this());
	socket_.async_connect(ep,
	                      [self](boost::system::error_code ec) {
		                      if (!ec) self->do_connectToPL();
		                      else self->stop();
	                      });
}

void Connection::start(ip::tcp::endpoint ep, string name) {
	m_name = name;
	do_connection(ep);
}

// 读取数据之后，解包，然后相应处理
// 消息格式是: 0xFFFF0000|0x0000FFFF,一个int32，其中前16位表示消息id，后面16位表示包体整体长度。后面跟着的是protobuf压缩过的数据
void Connection::do_read() {
	auto self(shared_from_this());
	socket_.async_read_some(
		buffer(read_buffer_ + already_read_, Max_DataBufferSize - already_read_),
		[this, self](boost::system::error_code ec, size_t length) {
			if (!ec) {
				already_read_ += length;
				while (already_read_ > PacketHeadLen) {
					int head = *reinterpret_cast<int *>(read_buffer_);
//					cout << "Head length: " << head << endl;
					uint16 msgid = (head & PacketMsgIdMask) >> 16;
					uint16 len = head & PacketMsgLenMask;
					if (len > Max_DataBufferSize || len < 0) {
						cerr << "头长度错误" << endl;
						already_read_ = 0;
						// 其实应该关闭连接
					}
					else if (already_read_ >= len) {
						string content(len, '\0');
						copy(read_buffer_ + PacketHeadLen, read_buffer_ + PacketHeadLen + len, content.begin());
						copy(read_buffer_ + PacketHeadLen + len, read_buffer_ + already_read_, read_buffer_); // 后面的数据没有清空
						already_read_ -= PacketHeadLen + len;
//						cout << "剩余读取长度： " << already_read_ << endl;

//						cout << "接收消息长度: " << PacketHeadLen + len << endl;
						//将msgid和content放入消息处理函数中
						if (!ConnectionMsgCenter::getInstance().dispatch(msgid, self, content.data(), len)) {
							cout << "消息没有处理: " << msgid << endl;
						}
					}
					else {
						cout << m_name << " 消息读取问题, msgid = " << msgid << " len = " << len << endl;
						break;
					}
				}

				do_read();
			}
			if (ec != error::operation_aborted) {
				// 断开连接
				// notice: 不需要断开连接，自己把自己坑死了
				cout << m_name << " boost::asio::error: " << boost::system::system_error(ec).what() << endl;
			}
		}
	);
}
