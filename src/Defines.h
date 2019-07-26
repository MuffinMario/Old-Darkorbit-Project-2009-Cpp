#ifndef DEFINES_H
#define DEFINES_H
#include <utility>
#include <string>
#include <iostream>
#include <thread>
#include <random>
#include <limits>

#include <boost\lexical_cast.hpp>

#include "coutcolor.h"

#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif
// I/O
using std::cout;
using std::cerr;
using std::clog;
#define dcout cout << EColor::LIGHT_BLUE << "[PID: " << std::this_thread::get_id() << "] "
#define warncout cout << EColor::YELLOW << "[Warning] "
#define BASIC_DEBUG_FUNCTION_COUT cout << EColor::PURPLE << __FUNCTION__ << " CALL!" << cendl;

// ASIO
#define BASIC_BIND_READER(Handler_) boost::bind(&Handler_::handle_read,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)
#define BASIC_BIND_WRITER(Handler_) boost::bind(&Handler_::handle_write,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)

// ERR
#define STD_CERR_OUTPUT(e) "[EXCEPTION] @std " << __FILE__ << ":" << __LINE__ << "(" << __FUNCTION__ << "): " << e.what()
#define BOOST_CERR_OUTPUT(e) "[EXCEPTION] @boost " << __FILE__ << ":" << __LINE__ << "(" << __FUNCTION__  << "): " << boost::diagnostic_information(e)

// TYPEDEF FOR CLEARANCE
typedef std::pair<int, int> Position_t; //simple
typedef int ore_t;
typedef int pos_t;
typedef unsigned short speed_t;
typedef int map_t;
typedef unsigned long long id_t;
typedef unsigned int config_t;
typedef unsigned short extraenergy_t;
typedef unsigned int collectable_t;
typedef unsigned int clanid_t;
typedef unsigned short shipid_t;
typedef unsigned int portalid_t;
typedef unsigned short factionid_t;
typedef unsigned int stationid_t;
typedef std::string username_t;
typedef std::string clan_t;
typedef std::string stationname_t;
typedef int shield_t;
typedef int health_t;
typedef int cargo_t;
typedef unsigned int ammo_t;
typedef unsigned int rocket_t;
typedef unsigned short weaponstate_t;
typedef unsigned long long exp_t;
typedef long long hon_t;
typedef unsigned short level_t;
typedef unsigned long long credits_t;
typedef unsigned long long uri_t;
typedef float jackpot_t;
typedef unsigned short rank_t;
typedef unsigned short gates_t;
typedef unsigned short lasertype_t;
typedef unsigned short rockettype_t;
typedef unsigned short portaltype_t;
typedef unsigned short clanstate_t;
typedef unsigned short stationtype_t;
typedef int damage_t;

std::ostream& operator<<(std::ostream& os, Position_t pos);
// Time in milliseconds
long long getTimeNow();
long long getTimeNowDelta(long long offset);
/* Returns true if timestamp+time is less than the current time in ms */
bool timeHasPassed(long long timeNow,long long timestamp, long long offset);
bool timeHasPassed(long long timestamp, long long offset);
bool timeHasPassed(long long timestamp);
template<typename T>
T random();
template<typename T>
T random(T max);
template<typename T>
T random(T min, T max);

/* Bool convert time after ~30 million rounds:
CPacketComposer::to_string(bool) : 1100ms
boost::lexical_cast<std::string>(bool) : 1600 ms
to_string(bool) : 8000-8500 ms
*/

template<typename T>
std::string to_string(T t);
std::string to_string(bool b);

std::string to_string(float t);
std::string to_string(double t);
std::string to_string(std::string& str);
std::string to_string(Position_t pos);

const id_t BEGIN_MOB_IDS = 1200000;
const id_t MOB_IDS_ADD_PER_MAP = 100000;
//mapid*10000 + 120000 + id
//max mobs: 10000

#include "Defines.tpp"

#endif
