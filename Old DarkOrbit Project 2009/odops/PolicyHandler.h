
#ifndef POLICYHANDLER_H
#define POLICYHANDLER_H
#include <iostream>
#include <boost\asio.hpp>
#include <boost\algorithm\string\predicate.hpp>
#include <boost\bind.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\enable_shared_from_this.hpp>

#include "FileReader.h"
#include "Defines.h"
#include "Constants.h"

//////////////////////////////////////////////////////////
//		Class PolicyHandler
//////////////////////////////////////////////////////////

typedef boost::asio::ip::tcp tcp_t;

// This class is derived from boost.org and some help of stackoverflow
class CPolicyHandler :
	public boost::enable_shared_from_this<CPolicyHandler>
{
	// Represents the socket
	tcp_t::socket socket_;
	//text to write
	std::string string_; // send
						 //received text
	std::string information;
	char buf[1024]; // received

	boost::asio::streambuf buffer;

	unsigned short m_port;
public:
	// Keeping it short
	typedef boost::shared_ptr<CPolicyHandler> tcppointer;

	// Creates a smart connection pointer to an object of this type.
	static tcppointer createConnection(boost::asio::io_service& io_service, unsigned short port);

	// Returns the socket
	tcp_t::socket& getSocket();

	// Starting to asynchrounously write data
	void start();
private:
	// We are going to use the ctor of our socket member
	CPolicyHandler(boost::asio::io_service& io_service, unsigned short& port) : socket_(io_service) { m_port = port; }

	void handle_write(const boost::system::error_code& ec, size_t bytes);

	void readData();
	void send();
	void disconnectUser();

	void handle_read(const boost::system::error_code & ec, size_t bytes);


};
#endif