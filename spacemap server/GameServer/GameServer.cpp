#include "GameServer.h"

void CGameServer::start()
{
	start_accept();
}

void CGameServer::start_accept()
{
	CPlayerHandler::tcppointer newconnection = CPlayerHandler::createConnection(io, m_port);

	acceptor_.async_accept(newconnection->getSocket(), boost::bind(&CGameServer::handle_accept, this, newconnection, boost::asio::placeholders::error));
}

void CGameServer::handle_accept(CPlayerHandler::tcppointer newconnection, const boost::system::error_code& err)
{
	if (!err) {
		std::string t = "[" + to_string(m_port) + "] Accepted connection " + newconnection->getSocket().remote_endpoint().address().to_string();
		
		dcout << t << cendl;

		newconnection->start();
	}
	//accept the next socket
	start_accept();
}
