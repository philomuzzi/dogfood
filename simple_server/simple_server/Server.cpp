#include "Server.h"
#include "PlayerConnection.h"


Server::~Server() {
}

void Server::do_accept() {
	acceptor_.async_accept(
		socket_,
		[this](boost::system::error_code ec) {
			if (!ec) {
				std::make_shared<PlayerConnection>(std::move(socket_))->start(); // is this a *swap* of socket_ ???
			}

			do_accept();
		}
	);
}
