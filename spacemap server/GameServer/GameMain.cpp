#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <thread>

#include "GameServerConfig.h"
CGameServerConfig g_gameServerConfig;

#include "GameServer.h"
#include "MovementManager.h"
#include "DBGetter.h"
#include "DBUpdater.h"
#include "ShipInfo.h"
#include <shellapi.h>
#include "BonusBox.h"
#include "../common/coutcolor.h"


extern std::map<map_t, CMap> g_allMaps;

extern CFileWriter g_filewrite;

extern std::map<level_t, exp_t> g_levelTable;
extern std::map<level_t, exp_t> g_droneLevelTable;

extern CDBGetter g_database_get;
extern CDBUpdater g_database_update;
extern std::map<shipid_t, CShipInfo> g_shipinfo;
extern std::map<rockettype_t, CRocketInfo> g_rocketinfo;
extern std::map<ore_t, COreInfo> g_oreinfo;
extern CSessionsManager g_sessionsManager;

namespace {

	void initSessions()
	{
		for (map_t m = 1; m <= 29; m++)
		{
			g_sessionsManager.createPermanentSession(m);
		}
		g_sessionsManager.createPermanentSession(42);
	}
	void initMaps()
	{
		g_allMaps = CMap::generateMapMap();
		for (auto& mappair : g_allMaps)
		{
			auto& map = mappair.second;
			//generate objects
			map.generateObjects();
		}
		cout << "All maps generated!" << std::endl;
	}
	void initBonusBoxes()
	{
		for (map_t m = 1; m <= 29; m++)
		{
			auto vec = CBonusBox::generateBonusBoxes(m, g_sessionsManager.getPermanentSession(m));
			for (std::shared_ptr<CBonusBox> bb : vec)
			{
				g_sessionsManager.getPermanentSession(m).addCollectable(bb);
			}
		}
		auto vec = CBonusBox::generateBonusBoxes(42, g_sessionsManager.getPermanentSession(42));
		for (std::shared_ptr<CBonusBox> bb : vec)
		{
			g_sessionsManager.getPermanentSession(42).addCollectable(bb);
		}
		cout << "All bonus boxes generated!" << std::endl;
	}
	void initMobs()
	{
		for (map_t m = 1; m <= 29; m++)
		{
			CSession& session = g_sessionsManager.getPermanentSession(m);
			std::vector<std::shared_ptr<CMob>> mobsInMap = CMob::generateMobsForMap(m, session);

			for (auto& mob : mobsInMap)
			{
				session.addMob(mob);

				//remember: do not add mob->spawn since the server is not even accepting sockets at this point
				// ^ response to this: doesnt play a role tbh
			}
		}

		// ??? map
		CSession& session = g_sessionsManager.getPermanentSession(42);
		std::vector<std::shared_ptr<CMob>> mobsInMap = CMob::generateMobsForMap(42, session);

		for (auto& mob : mobsInMap)
		{
			session.addMob(mob);
		}
		cout << "All aliens generated!" << std::endl;
	}
	void initLevelTable()
	{
		g_levelTable[0] = 0; //arrays start at zero not that this is an array, but to prevent any mistake for first starts etc....
		for (level_t level = 1; level <= 32; level++)
		{

			g_database_get.changeQuery("nave", "experiencias", "nivel", level);
			g_levelTable[level] = g_database_get.getUInt64();
			//dcout << "Level: " << level << " EP needed: " << g_levelTable[level] << cendl;
		}
		g_droneLevelTable[0] = 0; //arrays start at zero not that this is an array, but to prevent any mistake for first starts etc....
		for (level_t level = 1; level <= 6; level++)
		{

			g_database_get.changeQuery("vant", "experiencias", "nivel", level);
			g_droneLevelTable[level] = g_database_get.getUInt64();
			//dcout << "Level: " << level << " EP needed: " << g_levelTable[level] << cendl;
		}
	}
	void initVars()
	{
		initLevelTable();

		auto initShipInfo = [](shipid_t sid) {
			CShipInfo si;

			si.setShipId(sid);
			g_database_get.changeQuery("velocidad", "naves", "id", sid);
			si.setBaseSpeed(static_cast<speed_t>(g_database_get.getUInt64()));
			g_database_get.changeQuery("carga", "naves", "id", sid);
			si.setCargo(static_cast<cargo_t>(g_database_get.getUInt64()));
			g_database_get.changeQuery("vida", "naves", "id", sid);
			si.setBaseHP(static_cast<health_t>(g_database_get.getUInt64()));
			g_database_get.changeQuery("baterias", "naves", "id", sid);
			si.setBaseAmmo(static_cast<ammo_t>(g_database_get.getUInt64()));
			g_database_get.changeQuery("misiles", "naves", "id", sid);
			si.setBaseRockets(static_cast<rocket_t>(g_database_get.getUInt64()));
			return si;
		};
		auto initRocketInfo = [](rockettype_t rid) {
			CRocketInfo ri;

			ri.setType(rid);
			g_database_get.changeQuery("rocketname", "rockets", "rocketid", rid);
			ri.setName(g_database_get.getString());
			g_database_get.changeQuery("rocketdamage", "rockets", "rocketid", rid);
			ri.setDamage(static_cast<damage_t>(g_database_get.getUInt64()));
			g_database_get.changeQuery("rocketdistance", "rockets", "rocketid", rid);
			ri.setMaxDistance(static_cast<dist_t>(g_database_get.getUInt64()));
			return ri;
		};
		auto initOreInfo = [](ore_t oid) {
			COreInfo oi;

			oi.setOreType(oid);
			g_database_get.changeQuery("oreprice", "ores", "oreid", oid);
			oi.setCreditPrice(static_cast<credits_t>(g_database_get.getInt()));
			return oi;
		};



		for (shipid_t sid = 1; sid <= 10; sid++)
		{
			g_shipinfo[sid] = initShipInfo(sid);
		}
		g_shipinfo[50] = initShipInfo(50);
		g_shipinfo[52] = initShipInfo(52);
		g_shipinfo[53] = initShipInfo(53);

		//TODO: WIZ
		for (rockettype_t rid = 1; rid <= 3; rid++)
		{
			g_rocketinfo[rid] = initRocketInfo(rid);
		}


		for (ore_t oid = 1; oid <= 3; oid++)
		{
			g_oreinfo[oid] = initOreInfo(oid);
		}
		for (shipid_t oid = 11; oid <= 13; oid++)
		{
			g_oreinfo[oid] = initOreInfo(oid);
		}

		cout << "All variables initiated!" << std::endl;
	}
	void init()
	{
		/* RNG */
		initRandom();
		/* Global Variables (that I shouldnt be using to pollute the global space)*/
		initVars();
		/* Maps */
		initMaps();
		/* Permanent Sessions */
		initSessions();
		/* Collectables */
		initBonusBoxes();
		/* Aliens */
		initMobs();

	}
	void openWebBrowserTestSpacemap()
	{
		ShellExecute(0, 0, "http://localhost/internalSpacemapTestUnique.php?uid=1", 0, 0, SW_SHOW);
	}
	std::atomic<bool> alive = true;
	void fastTick()
	{
		// one loop averages on 1-2 ms on my pc (4 cores) and on a vserver with 1 core and 5 servers running ~18-25 ms ("Common KVM processor 2.4 GHz")
		// best option is split alien operations into 2 threads: moving and player interacting which differ in tick frequency
		// ^ i doubt that
		const unsigned int tickSleepWaitMS = 1000 / g_gameServerConfig->tickrate;
		while (alive)
		{
			g_sessionsManager.fastTick();
			std::this_thread::sleep_for(std::chrono::milliseconds(tickSleepWaitMS)); // 30 Hz tickrate DEFAULT
			/* Profiling
			CPU Specs:
			i5-4460 @ 3.20 GHz (quad-Core)
			Debug-Mode

			10 ms: 19%~21% CPU on n = 500
			50 ms: 13%~15% CPU on n = 500

			Release-Mode
			0 ms: constant 25% on n = 1500 (3x 500)
			10 ms: ~0%-4% CPU on n = 500 (4% on rare occasions though [spikes only])
			33 ms: 0%-1.2% CPU on n = ~1700
			50 ms: ~0% CPU on n = 500

			result: 30 Hz refresh rate nice and not shit cpu

			On average there should be about 30*50 NPCs (3x the amount of the test) on the server actively flying (30 maps ,ignoring personal GGs yet, inhabited by aliens)

			*/
		}
	}
	void secondTick()
	{
		while (alive)
		{
			g_sessionsManager.secondTick();
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	void cleanup()
	{
		// clean global variables
	}
	void game(unsigned short port) {
		bool restart = true;
		while (restart)
		{
			try {
				boost::asio::io_service io_service;

				CGameServer srv{ io_service, port };
				std::thread fastThread = std::thread(fastTick);
				std::thread secondThread = std::thread(secondTick);

				std::thread t([&restart, &io_service, &fastThread, &secondThread]() {
					std::cout << "> ";
					std::string str;
					std::getline(std::cin, str);
					if (str == "stop")
					{
						alive = false;
						restart = false;
						std::cout << "Stopping session threads" << std::endl;
						fastThread.join();
						secondThread.join();
						std::cout << "Stopping io_service" << std::endl;
						io_service.stop(); // io_service.stop stops the async_reads from sockets. Thus, there is no disconnectUser() call for errors, leaving etc.
						std::cout << "Clearing session (expects to disconnect users)" << std::endl;
						g_sessionsManager.clearAll(); // this saves their information in the database
						//disconnect from db

					}
					else if (str == "restart") {
						restart = true;
						alive = false;
						std::cout << "Stopping session threads" << std::endl;
						fastThread.join();
						secondThread.join();
						std::cout << "Stopping io_service" << std::endl;
						io_service.stop();
						g_sessionsManager.clearAll();
					}
					});
				io_service.run(); 
				t.join();
			}
			catch (boost::exception& e)
			{
				auto exc = boost::diagnostic_information(e);
				dcout << "Problem in main: " << exc << cendl;
				g_filewrite.writemore(exc, "ServEXC");
			}
		}
	}
}

void initConfig()
{
	try {
		g_gameServerConfig.loadFile(GAMESERVER_CONFIG_FILENAME);

		g_gameServerConfig.lookupData();
	}
	catch (const libconfig::FileIOException& fioe)
	{
		warncout << "Could not open " GAMESERVER_CONFIG_FILENAME "(Is this your first start?). Generating default config. (errmsg: " << fioe.what() << ")" << cendl;
		try { g_gameServerConfig.lookupData(); }
		catch (const CConfigNotAllValuesInitalizedException&) { /* do nothing, its expected */ }
		g_gameServerConfig.setData();
		g_gameServerConfig.writeFile(GAMESERVER_CONFIG_FILENAME);
	}
	catch (const CConfigNotAllValuesInitalizedException& ex)
	{
		warncout << "Not all config values are correctly set. Missing values will be reset to default. (errmsg: " << ex.what() << ")" << cendl;
		g_gameServerConfig.setData();
		g_gameServerConfig.writeFile(GAMESERVER_CONFIG_FILENAME);
	}
}


// SIDE NOTE: In a perfect world, I would do everything that requires asynchronous operations except sockets into one 
// single thread that polls events / moves NPCs on a 60 Hz base, but I am too lazy to reimplement all time based actions 
// and it doesnt seem to have such a huge CPU usage as 99% of CPU usage was the NPCs moving (which in total was 4%) until now (25.11.2017) (1.2% in Aug. 2018 :) )
int main(int argc, char** argv) {
	SetConsoleTitle("DarkOrbit 2009 GameServer");

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

	cout <<
		"Game Server with port " << g_gameServerConfig->port <<
		" set up." << endl;

	{
		bool connect_flag = false;
		do
		{
			cout << "Connecting to the database..." << endl;
			connect_flag = g_database_get.connect();
			//true on success, false on error
			if (connect_flag == false)
			{
				cout << "[DATABASE ERROR] Do you want to reconnect(Y/N): ";
				//std::cin.ignore(-1, '\n'); // because enter
				std::string answer;
				std::getline(std::cin, answer);
				if (answer.length() <= 0 || (answer.at(0) != 'Y' && answer.at(0) != 'y')) //more like start with "Y"/"y" but who cares anyways
				{
					//NO
					return -1;
				}
			}
			if (connect_flag == true) break; // internal compiler mistake apparenly -  sets connect_flag to false when its print was true and repeating this loop. weird
		} while (true);
	}
	cout << "Initalizing constant variables..." << endl;
	init();
	cout << "Starting server..." << endl;
	game(g_gameServerConfig->port);
	cout << "Cleaning up..." << endl;
	cleanup();
	system("PAUSE");
}