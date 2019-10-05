
#ifndef GAMEHANDLER_H
#define GAMEHANDLER_H

#include <iostream>
#include <map>
#include <thread>
#include <vector>
#include <array>
#include <future>
#include <set>
#include <ctime>
#include <random>
#include <memory>
#include <functional>

#include <boost\lexical_cast.hpp>
#include <boost\asio.hpp>
#include <boost\asio\steady_timer.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\algorithm\clamp.hpp>
#include <boost\math\special_functions\round.hpp>
#include <boost\bind.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\enable_shared_from_this.hpp>
#include <boost\thread.hpp>

#define NOMINMAX
#include <windows.h>

//#include "Session.h"
#include "SessionsManager.h"
#include "../common/FileReader.h"
#include "../common/FileWriter.h"
#include "../common/Defines.h"
#include "../common/coutcolor.h"
#include "../common/Constants.h"
#include "../common/PacketComposer.h"
#include "DamageManager.h"
#include "Enums.h"
#include "MovementManager.h"
#include "Player.h"
#include "DBGetter.h"
#include "DatabaseUtils.h"
#include "Mob.h"
#include "Collectable.h"
#include "ShipInfo.h"
#include "RocketInfo.h"
#include "OreInfo.h"
#include "../common/WorkerThreadPool.h"

typedef boost::asio::ip::tcp tcp_t;


//////////////////////////////////////////////////////////
//		Class PlayerHandler
//////////////////////////////////////////////////////////
// For further information check the boost.asio library
class CPlayerHandler :
	public IHandler,
	public std::enable_shared_from_this<CPlayerHandler>
{
	/* Basically a thread pool*/
	//boost::asio::thread_pool m_threadPool;
	CWorkerThreadPool m_workerThreads;
	//std::vector<boost::thread*>		m_asyncThreads;

	CDamageManager					m_damageManager;
	CSession*						m_currentSession = nullptr;


	/* The fact that there is a PNG packet, which gets sent every 20 or so seconds where 
	youre not sending any other packet means, that you should disconnect a user, when there is 
	no traffic coming from the user itself after a specific time (technically a soft-time out)
	practically it can also come as an auto afk kick tool, which kicks the user every so or so minutes (x PNG packets in a row)

	m_last_ping_time contains the last time in seconds where a user has sent a packet

	keep in mind that this does not ensure 100% afks getting kicked instead use a worker thread that disconnects on no action for x minutes
	*/
	std::atomic_llong m_last_ping_time = 0LL;

	std::atomic<bool>				m_bIsAttackThreadRunning = false;

	std::atomic<cfg_t>			m_cCurrentConfig = 1;
	bool							m_bLoginSent = false;

	lasertype_t	m_lasertype = 1; // todo move to m_player with v
	ERocketType	m_rockettype = ERocketType::R310; // inconsistency aw yes

	
	CPacketComposer m_sendpacket;
	// Represents the socket
	tcp_t::socket	m_socket;

	id_t			m_selectedOpponent = 0U;
	id_t			m_previousSelectedOpponent = 0U;
	long long		m_selectedTimeStamp = 0LL;
	//text to write
	std::string		m_information; //received text
	char			m_buf[2048] = { 0 }; // received buffer text

	boost::asio::streambuf	m_buffer;
	unsigned short			m_port = 0;

	std::vector<std::string> m_splitpackets;
public:
	~CPlayerHandler();
	CPlayerHandler(CPlayerHandler&& mov) = default;
	CPlayerHandler(const CPlayerHandler& cpy)	= default;

	typedef std::shared_ptr<CPlayerHandler> tcppointer;

	// Will be creating the connection
	static tcppointer createConnection(boost::asio::io_service& io_service, unsigned short port);

	// Returns the socket
	tcp_t::socket& getSocket();
	bool attackID(id_t uid);
	void handleConfigChange(unsigned int toConfig);
	void onEPChange() override;
	void selectOpponent(id_t id);
	// Starting to asynchrounously write data
	void start();
	bool login();
	bool sendPacket(const std::string& str) override;
	bool sendPacket(const char* str) override;
	bool sendPacketSynchronous(std::string& str) override;
	bool sendPacketSynchronous(const char* str) override;
	bool sendPackets(std::initializer_list<std::string> strs);
	void sendPacketAfter(int ms, std::string& str);
	void sendPacketAfter(int ms, const char* str);
	void sendEveryone(const std::string& packet);
	void sendEveryoneButMe(const std::string& packet);

	damage_t receiveDamagePure(damage_t dmg) override;
	damage_t receiveDamageHP(damage_t dmg) override;
	damage_t receiveDamageSHD(damage_t dmg) override;
	
	static bool authCertification(id_t id, std::string sessionID);
	
	void disconnectUser(bool updateDB = true);
	void die() override;
	void suspendThreads() override;
private:
	// We are going to use the ctor of our socket member
	CPlayerHandler(boost::asio::io_service& io_service, unsigned short& port, CSession* sm) :
		m_socket(io_service),
		m_currentSession(sm),
		m_workerThreads(UINT_MAX),
		IHandler()
	{
		
		m_port = port;
	}
	void cloak();
	void uncloak();
	//maybe add this shared pointer too
	void handleAttack(id_t) /*noexcept*/;
	void detonateSMB();
	void detonateISH(long long delta);
	//handling function once we sent something to (a) client(s)
	void handle_write(const boost::system::error_code& ec, size_t bytes);

	//handling function once we received something from a client
	void handle_read(const boost::system::error_code & ec, size_t bytes);
	void handle_read_b(size_t bytes);
	void onSpaceStationTradeRequest();
	void savePosToDB();
	//sends a packet that serves as a message to the client (that red text when you get EXP/CRED/etc. for example
	void sendMessagePacket(const std::string& text);

	//recursive method to repeatiately read packets of the client
	void readData();

	map_t getUserIDisOnMap(id_t id);
	//checks if the id is on the same map as the client
	bool checkSelectedID(id_t id);

	//lists of packets that are sent when a user logs in, also setting variables
	void loginPlayer();
	//lists of packets that are sent when the user jumps, also setting variables
	void jump(map_t wantedMapID, pos_t dest_x, pos_t dest_y);
	void setResourcePrices();

	//async use; logs out when time is done and no interruption
	void logoutHandle();
	void onTrade(ore_t oreid, ore_t oreamount);
	/*
		Database things that are refreshed when player left.
	*/
	void db_updateOnLeave();

	/*
		Returns the distance between the ships position and the given position
	*/
	dist_t distanceTo(Position_t pos);
	/*
		Returns the distance between the ships position and the given player/mob id
	*/
	dist_t distanceTo(id_t id);

	/*
		Adds SHD to the ship. Includes updating
	*/
	shield_t addSHD(shield_t shd) override;
	/*
		Adds HP to the ship. Includes updating
	*/
	health_t addHP(health_t hp) override;

	/****
	Definitions

	Update: If needed, refresh info internally, then visually update for the player(s)
	Refresh: Internally change the variables according to DB, etc.
	****/

	/*
		Updates the health (HP/SHD) of the player
	*/
	void updateHealth(damage_t dmg); 
	/*
		Updates the SHD of the player
	*/
	void updateShield(damage_t dmg);
	/*
		Updates the HP of the player
	*/
	void updateHitpoints(damage_t dmg);
	/*
		Updates the Speed of the player
	*/
	void updateSpeed(speed_t speed); 
	/*
		Updates the cargo of the player
	*/
	void updateCargo();
	/*
		Updates the CRD,URI,JP,...? of the player
	*/
	void updateAccount();
	/*
		Updates the drones of the player
	*/
	void updateDrones();
	/*
		Updates position and map of the player
	*/
	void db_updateLocation();

	void refreshCargo(); // all cargo stuff
	void refreshAccount(); //cred,uri,jp,...?

	//generate Gates, Stations, more
	void generateObjects(map_t mapid);

	void generatePortals();
	void generateStations();
	void generatePlayer();
	void generateAliens();
	void generateCollectables();
	void generatePlayer(id_t id);

	//checks if a gate is in sight to prepare jump stuff
	//also checks for a station, and later for a player
	void checkForObjectsToInteract() override;

	//utility function to determine if an object is in a radius to the player (its a circle)
	bool isInRangeCircle(int radius, pos_t objx, pos_t objy);
	//uitlity function to determine if an object is in a radius to the player(thats a square)
	bool isInRangeSquare(int radius, pos_t objx, pos_t objy);

	void initPlayerOnLogin();
	void initPlayerOnJump(bool updateLocation);

	void packetsOnLogin() {

	}
	void packetsOnJump() {

	}

	void leaveSession();
	void joinSession(map_t mapid);

	bool packetIs(const std::string& str);

	bool packetIsLevel(const std::string& str, size_t level);

	bool makeDamage(damage_t dmg, handlePtr enemy);

	bool makeDamage(damage_t dmg, std::shared_ptr<CMob> enemy);

	damage_t makeSAB(damage_t dmg, handlePtr enemy);

	damage_t makeSAB(damage_t dmg, std::shared_ptr<CMob> mob);

	void abortAttack();

	void handleAttackRequest(const id_t uid);

	//fixes a packet (mostly the null-terminator at the beginning of the string because of idk)
	void fixPacket();

	std::string fancyPrintName();

	//helper member functions to not always create threads in other functions
	template<class callable, class... arguments>
	void async_func(callable&&, arguments&&...);
	//helper member functions to not always create threads in other functions
	template<class callable, class... arguments>
	void async_func(int,callable&&, arguments&&...);

};
#endif
