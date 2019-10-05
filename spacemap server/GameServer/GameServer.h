#ifndef GAMESERVER_H
#define GAMESERVER_H
#include <iostream>
#include <boost\asio.hpp>
#include <boost\bind.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\enable_shared_from_this.hpp>

#include "PlayerHandler.h"
#include "../common/Constants.h"
#include "../common/coutcolor.h"

typedef boost::asio::ip::tcp tcp_t;
class CGameServer
{
	boost::asio::io_service& io;
	tcp_t::acceptor acceptor_;
	unsigned short	m_port;
public:
	/* default constructor: normal i/o service and a port where the server is running on */
	CGameServer(boost::asio::io_service& io_service, unsigned short& port) : io(io_service),acceptor_(io_service, tcp_t::endpoint(tcp_t::v4(), port)) {
		m_port = port;
		start();
	}
private:
	/* Start the asynchronous operations of the server */
	void start();
	/* Accepts the start from an incoming connection */
	void start_accept();

	/* Accept handler for binding */
	void handle_accept(CPlayerHandler::tcppointer newconnection, const boost::system::error_code& err);

};
#endif