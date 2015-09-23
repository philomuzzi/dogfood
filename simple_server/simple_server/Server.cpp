#include "Server.h"
#include "Player.h"


Server::~Server() {
}

void Server::do_accept() {
	acceptor_.async_accept(
		socket_,
		[this](boost::system::error_code ec) {
			if (!ec) {
				std::make_shared<Player>(std::move(socket_))->start(); // is this a *swap* of socket_ ???
			}

			do_accept();
		}
	);
}
