#include "PolicyHandler.h"

auto CPolicyHandler::createConnection(boost::asio::io_service & io_service, unsigned short port) -> CPolicyHandler::tcppointer
{
	return tcppointer(new CPolicyHandler(io_service, port));
}

tcp_t::socket & CPolicyHandler::getSocket()
{
	return socket_;
}

void CPolicyHandler::start()
{
	try {
		send();
	}
	catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
}

void CPolicyHandler::readData() {
	boost::asio::async_read(socket_, boost::asio::buffer(buf), boost::bind(&CPolicyHandler::handle_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void CPolicyHandler::send() {
	//policy file

	FileReader rd;
	//std::string policy = rd.getStr("C:\\xampp\\htdocs\\crossdomain.xml");
	std::string policy = Constants::Send::W_CROSSDOMAINPOLICY; //	"<?xml version=\"1.0\"?><cross-domain-policy xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://www.adobe.com/xml/schemas/PolicyFileSocket.xsd\"><allow-access-from domain=\"*\" to-ports=\"*\" secure=\"false\" /><site-control permitted-cross-domain-policies=\"master-only\" /></cross-domain-policy>";
	std::shared_ptr<std::vector<char>> ch(new std::vector <char>);

	//send to socket
	boost::asio::async_write(socket_,
		boost::asio::buffer(policy),
		boost::bind(&CPolicyHandler::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void CPolicyHandler::handle_write(const boost::system::error_code & ec, size_t bytes)
{ 
	std::cout << "[" << m_port << "] Sent cross-domain-policy to " << socket_.remote_endpoint().address().to_string() << std::endl;
}

void CPolicyHandler::disconnectUser() {
	//safely disconnect user by first blocking the stream and then close the socket
	socket_.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both);
	socket_.close();
	std::cout << "[" << m_port << "] Disconnected User " << socket_.remote_endpoint().address().to_string() << std::endl;
}

void CPolicyHandler::handle_read(const boost::system::error_code & ec, size_t bytes)
{
	std::string txt(buf);
	if (bytes > 0)
		std::cout << "[" << m_port << " PACKET/" << bytes << "] " << txt << std::endl;
	/*if (boost::algorithm::starts_with(txt, "<policy-file-request/>")) {
		send();
	}
	else {
		readData();
	}*/
}
