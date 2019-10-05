#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <thread>

#include "PolicyServerConfig.h"
CPolicyServerConfig g_policyServerConfig;

#include "PolicyServer.h"
#include "../common/coutcolor.h"
void policy(unsigned short port) {
	boost::asio::io_service io_service;
	CPolicyServer srv{ io_service, port };
	std::thread t([&io_service]() {
		std::cout << "> ";
		std::string str;
		std::getline(std::cin, str);
		if (str == "stop")
			io_service.stop();
		});
	io_service.run();
	t.join();
}



void initConfig()
{
	try {
		g_policyServerConfig.loadFile(POLICYSERVER_CONFIG_FILENAME);

		g_policyServerConfig.lookupData();
	}
	catch (const libconfig::FileIOException& fioe)
	{
		warncout << "Could not open " POLICYSERVER_CONFIG_FILENAME "(Is this your first start?). Generating default config. " << cendl;
		try { g_policyServerConfig.lookupData(); }
		catch (const CConfigNotAllValuesInitalizedException& ex) { /* do nothing, its expected */ }
		g_policyServerConfig.setData();
		g_policyServerConfig.writeFile(POLICYSERVER_CONFIG_FILENAME);
	}
	catch (const CConfigNotAllValuesInitalizedException& ex)
	{
		warncout << "Not all config values are correctly set. Missing values will be reset to default." << cendl;
		g_policyServerConfig.setData();
		g_policyServerConfig.writeFile(POLICYSERVER_CONFIG_FILENAME);
	}
}

int main(int argc, char** argv) {

	SetConsoleTitle("DarkOrbit 2009 PolicyServer");

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

	/* Initalize the config */
	initConfig();

	cout <<	"Policy Server with port " << g_policyServerConfig->port << " set up." << endl;

	cout << "Starting server..." << endl;
	policy(g_policyServerConfig->port);
	cout << "Endpoint reached." << endl;

}