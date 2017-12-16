#ifndef DEFINES_H
#define DEFINES_H
#include <utility>
#include <string>

#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif

//typedefs for a clear sight of what variable is which type
typedef std::pair<int, int> Position_t; //makes it simpler
typedef unsigned short ore_t;
typedef int pos_t;
typedef unsigned short speed_t;
typedef int map_t;
typedef unsigned int id_t;
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
typedef unsigned short cargo_t;
typedef unsigned int ammo_t;
typedef unsigned int rocket_t;
typedef unsigned short weaponstate_t;
typedef unsigned long long exp_t;
typedef unsigned long long hon_t;
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

#define STD_CERR_OUTPUT(e) "STD-Exception in file " << __FILE__ << "|" << __LINE__ << " function " << __FUNCTION__ << ": " << e.what()
#define BOOST_CERR_OUTPUT(e) "BOOST-Exception in file " << __FILE__ << "|" << __LINE__ << " function " << __FUNCTION__  << ": " << boost::diagnostic_information(e)


/*
#define STD_CERR_OUTPUT(e) "STD-Exception in file " << __FILE__ << "|" << __LINE__ << " function " << __FUNCTION__ << ": " << e.what()
#define BOOST_CERR_OUTPUT(e) "BOOST-Exception in file " << __FILE__ << "|" << __LINE__ << " function " << __FUNCTION__  << ": " << boost::diagnostic_information(e)
//#define ENABLE_ONLY_POLICY_SERVER // policy only debug
//#define ENABLE_ONLY_GAME_SERVER // gameserver only debug*/

#endif