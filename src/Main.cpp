//very ugly and cheesy but splitting both servers makes debugging easier
#define BUILD_BOTH_SERVERS (BUILD_POLICY_SERVER == 1 && BUILD_GAME_SERVER == 1)

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <thread>
#if BUILD_BOTH_SERVERS == 1
#include <future>
#endif

#include <boost\asio.hpp>

#if BUILD_GAME_SERVER == 1
#include "GameServer.h"
#include "MovementManager.h"
#include "Packeter.h"
#include "DBGetter.h"
#include "DBUpdater.h"
#include "ShipInfo.h"
#include <shellapi.h>
#include "BonusBox.h"

extern std::map<map_t, CMap> g_allMaps;

extern CFileWriter g_filewrite;

extern std::atomic_uint g_DBG_AITickRate;
extern std::map<level_t, exp_t> g_levelTable;
extern std::map<level_t, exp_t> g_droneLevelTable;

extern CDBGetter g_database_get;
extern CDBUpdater g_database_update;
extern std::map<shipid_t, CShipInfo> g_shipinfo;
extern std::map<rockettype_t, CRocketInfo> g_rocketinfo;
extern std::map<ore_t, COreInfo> g_oreinfo;
extern CSessionsManager g_sessionsManager;

namespace {
	//everyone leave and save your shit in database btw not imlpemented yet
	void __cdecl onExit()
	{
		dcout << "exiting: " << std::endl;
		g_sessionsManager.clearAll();
		std::cin.get();
	}
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
			std::vector<std::shared_ptr<CMob>> mobsInMap = CMob::generateMobsForMap(m,session);

			for (auto& mob : mobsInMap)
			{
				session.addMob(mob);

				//remember: do not add mob->spawn since the server is not even accepting sockets at this point
				// ^ response to this: doesnt play a role tbh
			}
		}

		// ??? map
		CSession& session = g_sessionsManager.getPermanentSession(42);
		std::vector<std::shared_ptr<CMob>> mobsInMap = CMob::generateMobsForMap(42,session);
		
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
			si.setBaseSpeed(g_database_get.getUInt64());
			g_database_get.changeQuery("carga", "naves", "id", sid);
			si.setCargo(g_database_get.getUInt64());
			g_database_get.changeQuery("vida", "naves", "id", sid);
			si.setBaseHP(g_database_get.getUInt64());
			g_database_get.changeQuery("baterias", "naves", "id", sid);
			si.setBaseAmmo(g_database_get.getUInt64());
			g_database_get.changeQuery("misiles", "naves", "id", sid);
			si.setBaseRockets(g_database_get.getUInt64());
			return si;
		};
		auto initRocketInfo = [](rockettype_t rid) {
			CRocketInfo ri;

			ri.setType(rid);
			g_database_get.changeQuery("rocketname", "rockets", "rocketid", rid);
			ri.setName(g_database_get.getString());
			g_database_get.changeQuery("rocketdamage", "rockets", "rocketid", rid);
			ri.setDamage(g_database_get.getUInt64());
			g_database_get.changeQuery("rocketdistance", "rockets", "rocketid", rid);
			ri.setMaxDistance(g_database_get.getUInt64());
			return ri;
		};
		auto initOreInfo = [](ore_t oid) {
			COreInfo oi;

			oi.setOreType(oid);
			g_database_get.changeQuery("oreprice", "ores", "oreid", oid);
			oi.setCreditPrice(g_database_get.getInt());
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
		/* Global Variables (that I shouldnt be using to pollute the global space)*/
		initVars();
		/* Permanent Sessions */
		initSessions();
		/* Maps */
		initMaps();
		/* Collectables */
		initBonusBoxes();
		/* Aliens */
		initMobs();

	}
	void openWebBrowserTestSpacemap()
	{
		ShellExecute(0, 0, "http://localhost/internalSpacemapTestUnique.php?uid=1", 0, 0, SW_SHOW);
	}
	void fastTick()
	{
		// one loop averages on 1-2 ms on my pc (4 cores) and on a vserver with 1 core and 5 servers running ~18-25 ms ("Common KVM processor 2.4 GHz")
		// best option is split alien operations into 2 threads: moving and player interacting which differ in tick frequency
		for (;;)
		{
			g_sessionsManager.fastTick();
			std::this_thread::sleep_for(std::chrono::milliseconds(g_DBG_AITickRate)); // 30 Hz tickrate DEFAULT
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
		for (;;)
		{
			g_sessionsManager.secondTick();
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	void game(unsigned short port) {
		while (true)
		{
			try {
				boost::asio::io_service io_service;

				CGameServer srv{ io_service, port };
				std::thread fastThread = std::thread(fastTick);
				std::thread secondThread = std::thread(secondTick);

				io_service.run();
				/* Threads are not joined / detached so they should terminate on std::thread::~thread(), unsure about io_service*/
			}
			catch (boost::exception& e)
			{
				auto exc = boost::diagnostic_information(e);
				dcout << "Problem in main: " << exc << cendl;
				g_filewrite.writemore(exc,"ServEXC");
			}
			dcout << "Restarting server" << cendl;
		}
		/* threads die or not this is infinite anyways*/
	}

	static bool isInArgs(int argc, char** argv, char* text) {
		for (int i = 0; i < argc; i++) {
			if (strcmp(text, argv[i]) == 0)
				return true;
		}
		return false;
	}
}

#endif

#if BUILD_POLICY_SERVER == 1
#include "PolicyServer.h"

void policy(unsigned short port) {
	boost::asio::io_service io_service;
	CPolicyServer srv{ io_service, port };
	io_service.run();
}

#endif





// SIDE NOTE: In a perfect world, I would do everything that requires asynchronous operations except sockets into one 
// single thread that polls events / moves NPCs on a 60 Hz base, but I am too lazy to reimplement all time based actions 
// and it doesnt seem to have such a huge CPU usage as 99% of CPU usage was the NPCs moving (which in total was 4%) until now (25.11.2017) (1.2% in Aug. 2018 :) )
int main(int argc, char** argv) {

#if BUILD_BOTH_SERVERS == 1
	SetConsoleTitle("DarkOrbit 2009 Game-/PolicyServer");
#elif BUILD_POLICY_SERVER == 1
	SetConsoleTitle("DarkOrbit 2009 PolicyServer");
#elif BUILD_GAME_SERVER == 1
	SetConsoleTitle("DarkOrbit 2009 GameServer");
#else
	SetConsoleTitle("DarkOrbit 2009 Nothing");
#endif
#if BUILD_POLICY_SERVER == 1
	unsigned short policy_port = 843;
#endif
#if BUILD_GAME_SERVER == 1
	unsigned short game_port = 5152;
#endif

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
	
	cout << 
#if BUILD_POLICY_SERVER == 1
		"Policy Server with port " << policy_port <<
	#if BUILD_GAME_SERVER == 1
		" and " << 
	#endif
#endif
#if BUILD_GAME_SERVER == 1
		"Game Server with port " << game_port << 
#endif
		" set up." << endl;
#if BUILD_GAME_SERVER == 1
	bool skipenter = isInArgs(argc, argv, "fakoff");
	if(!skipenter) {
		cout << "Start Servers and connect to Database by pressing ENTER..." << endl;
		cout << "Also, if you don't want to press enter everytime, just add a -fakoff to the properties of the file" << endl;
		std::cin.get();
	}

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
				if (answer.length() <= 0 || ( answer.at(0) != 'Y' && answer.at(0) != 'y')) //more like start with "Y"/"y" but who cares anyways
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
#endif

	cout << "Starting server..." << endl;
#if BUILD_BOTH_SERVERS == 1 
	auto policy_server = std::async(policy,policy_port);
	auto game_server = std::async(game, game_port);
	try {
		//openWebBrowserTestSpacemap();
		policy_server.get();
		game_server.get();
	}
	catch (const std::exception& ex) {
		std::cerr << "ERROR in " << __FUNCTION__ << ": " << STD_CERR_OUTPUT(ex) << endl;
	}
	catch (const boost::exception& ex) {
		std::cerr << "ERROR in " << __FUNCTION__ << ": " << BOOST_CERR_OUTPUT(ex) << endl;
	}
#elif BUILD_POLICY_SERVER == 1 
	policy(policy_port);
#elif BUILD_GAME_SERVER == 1
	game(game_port);
#else
#error You built this project wrong. Decide if you want to build the policy or the game server or both
#endif
	cout << "Endpoint reached." << endl;

	system("PAUSE");
}