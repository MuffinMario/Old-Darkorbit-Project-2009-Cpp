#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <thread>
#include <future>

#include <boost\asio.hpp>

#include "GameServer.h"
#include "PolicyServer.h"
#include "MovementManager.h"
#include "Packeter.h"
#include "DBGetter.h"

namespace {
	bool isInArgs(int argc, char** argv, char* text) {
		for (int i = 0; i < argc; i++) {
			if (strcmp(text, argv[i]) == 0)
				return true;
		}
		return false;
	}


	void policy(unsigned short port) {
		boost::asio::io_service io_service;
		CPolicyServer srv{ io_service, port };
		io_service.run();
	}
	void game(unsigned short port) {
		boost::asio::io_service io_service;
		CGameServer srv{ io_service, port };
		io_service.run();
	}
}

int main(int argc, char** argv) {
	bool skipenter = isInArgs(argc, argv, "fakoff");

	unsigned short policy_port = 843;
	unsigned short game_port = 5152;

	using std::cout;
	using std::endl;
	/* Fancy dancy greeting */
	cout << "\t\t-----------------------------------------" << endl;
	cout << "\t\t|        ~+\t\t\t'*\t|\tO" << endl;
	cout << "\tP\t|                 *       +\t\t|" << endl;
	cout << "\t\t|           '               |\t\t|\tR" << endl;
	cout << "\tO\t|       ()    .-.,=\"``\"=. - o - \t|" << endl;
	cout << "\t\t|  P O O P    '=/_       \\  |\t\t|\tB" << endl;
	cout << "\tO\t|          *   |  '=._    |\t\t|" << endl;
	cout << "\t\t|               \\     `=./`,   \t\t|\tI" << endl;
	cout << "\tP\t|            .   '=.__.=' `='      *\t|" << endl;
	cout << "\t\t|   +           O R B I T    +\t\t|\tT" << endl;
	cout << "\t\t|        O      *        '       .\t|" << endl;
	cout << "\t\t-----------------------------------------" << endl;
	
	cout << "Policy Server with port " << policy_port << " and Game Server with port " << game_port << " set up." << endl;

	if(!skipenter) {
		cout << "Start Servers and connect to Database by pressing ENTER..." << endl;
		cout << "Also, if you don't want to press enter everytime, just add a -fakoff to the properties of the file" << endl;
		std::cin.get();
	}

	cout << "Connecting to the database..." << endl;
	database_get.connect();

	auto policy_server = std::async(policy,policy_port);
	auto game_server   = std::async(game,  game_port);

	try {
		policy_server.get();
		game_server.get();
	}
	catch (const std::exception& ex) {
		std::cerr << "ERROR in " << __FUNCTION__ << ": " << STD_CERR_OUTPUT(ex) << endl;
	}
	catch (const boost::exception& ex) {
		std::cerr << "ERROR in " << __FUNCTION__ << ": " << BOOST_CERR_OUTPUT(ex) << endl;
	}
	cout << "Thread(s) terminated!" << endl;
	system("PAUSE");
}