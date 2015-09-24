#pragma once

#include <memory>
#include <boost/asio.hpp>

class PlayerConnection : public std::enable_shared_from_this<PlayerConnection> {
public:
	const static int PacketHeadLen = sizeof(int32_t);
	const static int PacketMsgIdMask = 0x7fff0000;
	const static int PacketMsgLenMask = 0x0000ffff;
	const static int Max_DataBufferSize = 64 * 1024 - 1;
	const static int Max_DataSize = Max_DataBufferSize - PacketHeadLen;
	const static int Max_UserDataSize = Max_DataSize - 128;

public:
	explicit PlayerConnection(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}

	void start();
	void write(std::string data, std::size_t length);
	void sendCmdMsg(std::string data, int head);


private:
	void do_read();

	boost::asio::ip::tcp::socket socket_;
	char read_buffer_[Max_DataBufferSize];
	std::array<char, Max_DataBufferSize> write_buffer_;

	int already_read_{ 0 };
};