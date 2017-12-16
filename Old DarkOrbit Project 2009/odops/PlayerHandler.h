
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

#include <windows.h>

#include "SessionsManager.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "Defines.h"
#include "Constants.h"
#include "PacketComposer.h"
#include "DamageManager.h"
#include "Map.h"
#include "Enums.h"
#include "MovementManager.h"
#include "Player.h"
#include "DBGetter.h"
#include "DatabaseUtils.h"
#include "Mob.h"

typedef boost::asio::ip::tcp tcp_t;


//////////////////////////////////////////////////////////
//		Class PlayerHandler
//////////////////////////////////////////////////////////
// For further information check the boost.asio library
class CPlayerHandler :
	public IHandler,
	public std::enable_shared_from_this<CPlayerHandler>
{
	std::vector<boost::thread*>		m_asyncThreads;

	CDamageManager					m_damageManager;
	CSessionsManager*				m_currentSession;
	std::thread*					m_laser_thread; // unused atm


	/* The fact that there is a PNG packet, which gets sent every 20 or so seconds where 
	youre not sending any other packet means, that you should disconnect a user, when there is 
	no traffic coming from the user itself after a specific time (a so called timeout)
	practically it can also come as an auto afk kick tool, which kicks the user every so or so minutes (x PNG packets in a row)

	m_last_ping_time contains the last time in seconds where a user has sent a packet
	*/
	std::atomic_llong m_last_ping_time;

	std::atomic<bool>				m_bIsAttackThreadRunning;
	std::atomic<bool>				m_bIsRepairThreadRunning;
	bool							m_bLoginSent;

	lasertype_t	m_lasertype;
	ERocketType	m_rockettype;

	
	CPacketComposer m_sendpacket;
	// Represents the socket
	tcp_t::socket	m_socket;
	id_t			m_selectedOpponent;
	//text to write
	std::string		m_information; //received text
	char			m_buf[2048]; // received buffer text

	boost::asio::streambuf	m_buffer;
	unsigned short			m_port;

	std::vector<std::string> m_splitpackets;
public:
	static std::map<int, CSessionsManager> m_sAllInSession; //each map has its own session which has its own PlayerHandlers
	~CPlayerHandler();
	CPlayerHandler(CPlayerHandler&& mov) = default;
	CPlayerHandler(const CPlayerHandler& cpy)	= default;

	typedef std::shared_ptr<CPlayerHandler> tcppointer;

	// Will be creating the connection
	static tcppointer createConnection(boost::asio::io_service& io_service, unsigned short port);

	// Returns the socket
	tcp_t::socket& getSocket();

	// Starting to asynchrounously write data
	void start();
	void login();
	bool sendPacket(std::string str);
	bool sendPackets(std::vector<std::string> strs);
	void sendPacketAfter(int ms, std::string str);
	static void sendEveryone(std::string packet,map_t map_id);

	damage_t receiveDamagePure(damage_t dmg);
	damage_t receiveDamageHP(damage_t dmg);
	damage_t receiveDamageSHD(damage_t dmg);

	void die();
	void flushThreads();
private:
	// We are going to use the ctor of our socket member
	CPlayerHandler(boost::asio::io_service& io_service, unsigned short& port, CSessionsManager* sm) :
		m_socket(io_service),
		m_currentSession(sm),
		IHandler()
	{
		m_port = port;
	}

	//maybe add this shared pointer too
	void handleAttack(id_t) /*noexcept*/;
	void detonateSMB();
	void detonateISH();
	//handling function once we sent something to (a) client(s)
	void handle_write(const boost::system::error_code& ec, size_t bytes);

	//handling function once we received something from a client
	void handle_read(const boost::system::error_code & ec, size_t bytes);
	void handle_read_b(size_t bytes);
	void savePosToDB();
	//sends a packet that serves as a message to the client (that red text when you get EXP/CRED/etc. for example
	void sendMessagePacket(std::string text);
	//unused, not working
	void disconnectUser();

	//recursive method to repeatiately read packets of the client
	void readData();

	map_t getUserIDMapId(id_t id);
	//checks if the id is on the same map as the client
	bool checkSelectedID(id_t id);

	//lists of packets that are sent when a user logs in, also setting variables
	void logInPackets();
	//lists of packets that are sent when the user jumps, also setting variables
	void jump(map_t wantedMapID, pos_t dest_x, pos_t dest_y);

	//async use; logs out when time is done and no interruption
	void logoutHandle();

	//update by CPlayerInfo
	void updateHealth(); //hp,shd
	void updateShield();
	void updateHitpoints();
	void updateAccount(); //cred,uri,jp
	void updateSpeed(); //spd
	
	//generate Gates, Stations, more
	void generateObjects(map_t mapid);

	void generatePortals();
	void generateStations();
	void generatePlayer();
	void generateAliens();
	void generatePlayer(id_t id);

	//checks if a gate is in sight to prepare jump stuff
	//also checks for a station, and later for a player
	void checkForObjectsToInteract();

	//utility function to determine if an object is in a radius to the player (its a circle)
	bool isInRangeCircle(int radius, pos_t objx, pos_t objy);
	//uitlity function to determine if an object is in a radius to the player(thats a square)
	bool isInRangeSquare(int radius, pos_t objx, pos_t objy);


	bool packetIs(std::string str);

	bool packetIsLevel(std::string str, size_t level);

	bool makeDamage(damage_t dmg, handlePtr enemy);

	bool makeDamage(damage_t dmg, std::shared_ptr<CMob> enemy);

	damage_t makeSAB(damage_t dmg, handlePtr enemy);

	damage_t makeSAB(damage_t dmg, std::shared_ptr<CMob> mob);

	void handleAttackRequest(const id_t uid);

	//fixes a packet (mostly the null-terminator at the beginning of the string because of idk)
	void fixPacket();

	//helper member functions to not always create threads in other functions
	template<class callable, class... arguments>
	boost::thread* async_func(callable&&, arguments&&...);
	//helper member functions to not always create threads in other functions
	template<class callable, class... arguments>
	boost::thread* async_func(int,callable&&, arguments&&...);

#define A
#ifdef A
	void DBG_SIMULATE_AI();
	void DBG_SIMULATE_AI_MOVE();
#endif

};
#endif
