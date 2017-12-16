#include "PolicyHandler.h"

CPolicyHandler::~CPolicyHandler()
{
	std::cout << "[" << m_port << "] Socket disconnected!" << std::endl;
}

auto CPolicyHandler::createConnection(boost::asio::io_service & io_service, unsigned short port) -> CPolicyHandler::tcppointer
{
	return tcppointer(new CPolicyHandler(io_service, port));
}

tcp_t::socket & CPolicyHandler::getSocket()
{
	return m_socket;
}

void CPolicyHandler::start()
{
	try {
		readData();
	}
	catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
}

void CPolicyHandler::readData() {

	dcout << "use_counts: " << shared_from_this().use_count() << cendl;
	m_socket.read_some(boost::asio::buffer(buf));
	std::string txt(buf);
	if (txt.size() > 0)
	{
		if (boost::algorithm::starts_with(txt, "<policy-file-request/>")) {
		std::cout << "[" << m_port << "] received Policy File Request" << std::endl;
			send();
		}
	}
	else {
		disconnectUser();
	}
}

void CPolicyHandler::send() {
	//policy file

	CFileReader rd;
	//std::string policy = rd.getStr("C:\\xampp\\htdocs\\crossdomain.xml");
	std::string policy = Constants::Send::W_CROSSDOMAINPOLICY; //	"<?xml version=\"1.0\"?><cross-domain-policy xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://www.adobe.com/xml/schemas/PolicyFileSocket.xsd\"><allow-access-from domain=\"*\" to-ports=\"*\" secure=\"false\" /><site-control permitted-cross-domain-policies=\"master-only\" /></cross-domain-policy>";

															   //send to socket
															   //socket_.write_some(boost::asio::buffer(policy));
	/*boost::asio::async_write(socket_,
		boost::asio::buffer(policy),
		boost::bind(&CPolicyHandler::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));*/
	boost::asio::streambuf sendbuffer;
	std::ostream os(&sendbuffer);
	os << policy;
	boost::asio::write(m_socket,
		sendbuffer.data()); // not async because every player is one thread and sending doesn't take forever -> sendPacket gets called from a player or an async thread from a player

}

void CPolicyHandler::handle_write(const boost::system::error_code & ec, size_t bytes)
{
	std::cout << "[" << m_port << "] Sent cross-domain-policy to " << m_socket.remote_endpoint().address().to_string() << std::endl;
}

void CPolicyHandler::disconnectUser() {
	//safely disconnect user by first blocking the stream and then close the socket
	m_socket.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both);
	m_socket.close();
	std::cout << "[" << m_port << "] Disconnected User " << m_socket.remote_endpoint().address().to_string() << std::endl;
}

void CPolicyHandler::handle_read(const boost::system::error_code & ec, size_t bytes)
{
	std::string txt(buf);
	if (bytes > 0)
		std::cout << "[" << m_port << " PACKET/" << bytes << "] " << txt << std::endl;
	if (boost::algorithm::starts_with(txt, "<policy-file-request/>")) {
		//yeah this happens
		send();
	}
	else {
		disconnectUser();
	}
}
