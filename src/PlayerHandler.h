
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

//#include "Session.h"
#include "SessionsManager.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "Defines.h"
#include "Constants.h"
#include "PacketComposer.h"
#include "DamageManager.h"
#include "Enums.h"
#include "MovementManager.h"
#include "Player.h"
#include "DBGetter.h"
#include "DatabaseUtils.h"
#include "Mob.h"
#include "Collectable.h"
#include "ShipInfo.h"

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
	CSession*						m_currentSession = nullptr;
	std::thread*					m_laser_thread; // unused atm


	/* The fact that there is a PNG packet, which gets sent every 20 or so seconds where 
	youre not sending any other packet means, that you should disconnect a user, when there is 
	no traffic coming from the user itself after a specific time (technically a soft-time out)
	practically it can also come as an auto afk kick tool, which kicks the user every so or so minutes (x PNG packets in a row)

	m_last_ping_time contains the last time in seconds where a user has sent a packet

	keep in mind that this does not ensure 100% afks getting kicked instead use a worker thread that disconnects on no action for x minutes
	*/
	std::atomic_llong m_last_ping_time = 0LL;

	std::atomic<bool>				m_bIsAttackThreadRunning = false;

	std::atomic<config_t>			m_cCurrentConfig = 1;
	bool							m_bLoginSent = false;

	lasertype_t	m_lasertype = 1;
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
	void login();
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

	void disconnectUser();
	void die() override;
	void suspendThreads() override;
private:
	// We are going to use the ctor of our socket member
	CPlayerHandler(boost::asio::io_service& io_service, unsigned short& port, CSession* sm) :
		m_socket(io_service),
		m_currentSession(sm),
		m_laser_thread(nullptr),
		IHandler()
	{
		m_port = port;
	}

	void cloak();
	void uncloak();
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
	void sendMessagePacket(const std::string& text);

	//recursive method to repeatiately read packets of the client
	void readData();

	map_t getUserIDisOnMap(id_t id);
	//checks if the id is on the same map as the client
	bool checkSelectedID(id_t id);

	//lists of packets that are sent when a user logs in, also setting variables
	void logInPackets();
	//lists of packets that are sent when the user jumps, also setting variables
	void jump(map_t wantedMapID, pos_t dest_x, pos_t dest_y);

	//async use; logs out when time is done and no interruption
	void logoutHandle();

	shield_t addSHD(shield_t shd) override;
	health_t addHP(health_t hp) override;
	//update by CPlayerInfo
	void updateHealth(damage_t dmg); //hp,shd
	void updateShield(damage_t dmg);
	void updateHitpoints(damage_t dmg);
	void updateAccount(); //cred,uri,jp
	void updateSpeed(speed_t speed); //spd
	
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
