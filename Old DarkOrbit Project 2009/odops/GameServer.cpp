#include "GameServer.h"

void CGameServer::start_accept()
{
	CPlayerHandler::tcppointer newconnection = CPlayerHandler::createConnection(acceptor_.get_io_service(), m_port);

	acceptor_.async_accept(newconnection->getSocket(), boost::bind(&CGameServer::handle_accept, this, newconnection, boost::asio::placeholders::error));
}

void CGameServer::handle_accept(CPlayerHandler::tcppointer newconnection, const boost::system::error_code& err)
{
	if (!err) {
		std::string t = "[" + std::to_string(m_port) + "] Accepted connection " + newconnection->getSocket().remote_endpoint().address().to_string();
		
		std::cout << t << std::endl;

		newconnection->start();

		std::cout << "ravioli ravioli, give me the reasonoli" << std::endl;

		//accept the next socket
		start_accept();
	}
}
