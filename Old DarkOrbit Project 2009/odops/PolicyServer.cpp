#include "PolicyServer.h"

void CPolicyServer::start_accept()
{
	CPolicyHandler::tcppointer newconnection = CPolicyHandler::createConnection(acceptor_.get_io_service(), m_port);

	acceptor_.async_accept(newconnection->getSocket(), boost::bind(&CPolicyServer::handle_accept, this, newconnection, boost::asio::placeholders::error));
}

void CPolicyServer::handle_accept(CPolicyHandler::tcppointer newconnection, const boost::system::error_code& err)
{
	if (!err) {
		std::cout << "[" << m_port << "] Accepted connection " << newconnection->getSocket().remote_endpoint().address().to_string() << std::endl;
		newconnection->start();

		//accept the next socket
		start_accept();
	}
}
