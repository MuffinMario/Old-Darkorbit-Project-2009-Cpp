
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
#include <boost\algorithm\string.hpp>
#include <boost\bind.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\enable_shared_from_this.hpp>
#include <boost\thread.hpp>

#include <mysql_connection.h>
#include <mysql_error.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

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

typedef boost::asio::ip::tcp tcp_t;


//////////////////////////////////////////////////////////
//		Class PlayerHandler
//////////////////////////////////////////////////////////
// For further information check the boost.asio library
class CPlayerHandler :
	public Handler,
	public boost::enable_shared_from_this<CPlayerHandler>
{
	std::mutex						mutex;
	std::vector<std::thread*>		asyncThreads;

	DamageManager					damageManager;
	SessionsManager&				currentSession;
	std::shared_ptr<std::thread>	laser_thread;

	std::atomic<bool>				isAttackThreadRunning;
	std::atomic<bool>				isRepairThreadRunning;

	LaserType	lasertype;
	RocketType	rockettype;

	
	PacketComposer sendpacket;

	FileWriter		write;
	// Represents the socket
	tcp_t::socket	socket_;
	id_t			selectedOpponent;
	//text to write
	std::string		information; //received text
	char			buf[1024]; // received buffer text

	boost::asio::streambuf	buffer;
	unsigned short			m_port;
public:
	~CPlayerHandler() {
		for (int i = 0; i < asyncThreads.size();++i) {
			std::thread* current_thread = asyncThreads.at(i);
			if (current_thread != nullptr)
				delete current_thread;
			else
				std::cout << "thread may actually be nullptr" << std::endl;
		}
		currentSession.leaveSession(shared_from_this());
		std::cout << "\t!!! -> " << id << " cleared" << std::endl;
	}
	CPlayerHandler(CPlayerHandler&&)		= default;
	CPlayerHandler(const CPlayerHandler&)	= default;

	static std::map<int, SessionsManager> allInSession; //each map has its own session which has its own PlayerHandlers
	typedef boost::shared_ptr<CPlayerHandler> tcppointer;

	// Will be creating the connection
	static tcppointer createConnection(boost::asio::io_service& io_service, unsigned short port);

	// Returns the socket
	tcp_t::socket& getSocket();

	// Starting to asynchrounously write data
	void start();
	void sendPacket(std::string str);
	void sendPacketAfter(int ms, std::string str);
	void sendEveryone(std::string packet);

	void receiveDamagePure(damage_t dmg, id_t from);
	void receiveDamageHP(damage_t dmg, id_t from);
	void receiveDamageSHD(damage_t dmg, id_t from);

	void die();
private:
	// We are going to use the ctor of our socket member
	CPlayerHandler(boost::asio::io_service& io_service, unsigned short& port, SessionsManager& sm) :
		socket_(io_service),
		write("C:/xampp/htdocs/log.txt", m_port),
		currentSession(sm),
		Handler()
	{
		m_port = port;	
	}

	//maybe move this to damagemanager, add this shared pointer too
	void handleAttack(id_t) noexcept;
	void detonateSMB();
	void detonateISH();
	//handling function once we sent something to (a) client(s)
	void handle_write(const boost::system::error_code& ec, size_t bytes);

	//handling function once we received something from a client
	void handle_read(const boost::system::error_code & ec, size_t bytes);

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

	//update by PlayerInfo
	void updateHealth(); //hp,shd
	void updateAccount(); //cred,uri,jp
	void updateSpeed(); //spd
	
	//generate Gates, Stations, more
	void generateObjects(map_t mapid);

	void generatePortals();
	void generateStations();
	void generatePlayer();
	void generatePlayer(id_t id);

	//checks if a gate is in sight to prepare jump stuff
	//also checks for a station, and later for a player
	void checkForObjectsToInteract();

	//utility function to determine if an object is in a radius to the player (its a circle)
	bool isInRangeCircle(int radius, pos_t objx, pos_t objy);
	//uitlity function to determine if an object is in a radius to the player(thats a square)
	bool isInRangeSquare(int radius, pos_t objx, pos_t objy);


	bool packetStartsWith(std::string str);

	bool makeDamage(damage_t dmg, id_t from, handlePtrIt it);

	void handleAttackRequest(const id_t uid);

	//fixes a packet (mostly the null-terminator at the beginning of the string because of idk)
	void fixPacket();

	//helper member functions to not always create threads in other functions
	template<class callable, class... arguments>
	std::thread* async_func(callable&&, arguments&&...);
	//helper member functions to not always create threads in other functions
	template<class callable, class... arguments>
	std::thread* async_func(int,callable&&, arguments&&...);

	/*simple and strict, maybe i should put that somewhere else
	std::string getUsername(id_t id = 0);
	factionid_t getCompany(id_t id = 0);
	shipid_t getShip(id_t  id = 0);
	rank_t getRank(id_t id = 0);
	Position_t getPos(id_t id = 0);
	map_t getMap(id_t id = 0);
	health_t getHP(id_t id = 0);
	health_t getmaxHP(id_t id = 0);
	shield_t getSHD(id_t id = 0);
	shield_t getmaxSHD(id_t id = 0);
	speed_t getSpeed(id_t id = 0);

	level_t getLevel();
	cargo_t getCargo();
	exp_t getEXP();
	hon_t getHON();
	credits_t getCRD();
	uri_t getURI();
	jackpot_t get JP();*/

};
#endif
