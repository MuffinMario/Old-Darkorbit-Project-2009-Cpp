
#ifndef PACKETCOM_H
#define PACKETCOM_H
#include "Defines.h"
#include <string>
#include <limits>
#include <iostream>
#include <deque>
#include <cmath>
#include <chrono>
#include <boost\lexical_cast.hpp>

#include "Packeter.h"
#include "Constants.h"
//////////////////////////////////////
//		PacketComposer (i dont know how i could name this good)
//	will always return a string which builds up a packet
/////////////////////////////////////
class PacketComposer {
	/* Bool convert time after ~30 million rounds:
		PacketComposer::to_string(bool) : 1100ms
		boost::lexical_cast<std::string>(bool) : 1600 ms
		std::to_string(bool) : 8000-8500 ms
	*/

	//variables
	std::deque<std::string> i;

	//methods
	template<typename T>
	void push(T t);
	template<typename T, typename ... O>
	void push(T t, O... o);
	template<typename T, typename ... O>
	std::deque<std::string> generateStringDeque(T t, O... other);

	std::string roundFloatToString(float number, size_t decimalplace);

	template<typename T>
	std::string to_string(T t);
	std::string to_string(bool b);
	std::string to_string(std::string str);
public:
	//ERR
	std::string error_noHP();
	std::string error_notLoggedIn();
	std::string error_alreadyLoggedIn();
	std::string error_invalidSession();

	std::string move(id_t userID_, pos_t lx, pos_t ly, pos_t px, pos_t py ,speed_t speed,long long ms);

	std::string events(bool inDemizone, bool playRepanim, bool inTradezone, bool inRadiationzone, bool inJumpzone, bool repvouchs, pos_t x, pos_t y);
	std::string events(bool inDemizone, bool playRepanim, bool inTradezone, bool inRadiationzone, bool inJumpzone, bool repvouchs);

	std::string laserAttack(id_t from, id_t to, lasertype_t lasercolor);
	std::string laserAbort(id_t from,id_t to);
	std::string rocketAttack(id_t from, id_t to, rockettype_t rocketType, bool hit = true);

	std::string init(id_t userID, username_t username, shipid_t shipID, speed_t speed, shield_t shield,
		shield_t shieldmax, health_t health, health_t healthmax, cargo_t cargospace, cargo_t cargospacemax, pos_t x, pos_t y,
		map_t mapID, factionid_t fractionid, clanid_t clanid, ammo_t battmax, rocket_t rocketsmax, weaponstate_t oState,
		bool isPremium, exp_t experience, hon_t honour, level_t level, credits_t credits, uri_t uridium,
		jackpot_t jackpot, rank_t userrank, clan_t clantag, gates_t gatesAchieved, bool useSysFont);
	std::string loadMap(map_t mapid);
	std::string loadMiniMap(map_t mapid);
	std::string spawnEnemy(id_t userID, shipid_t shipID, weaponstate_t oState, clan_t clan, username_t username, pos_t x, pos_t y, factionid_t company, clanid_t clanID, rank_t rank, bool showRedSquareOnMinimap, clanstate_t clanDiplo, gates_t galaxygates, bool usebigfont = 1);
	//maybe do spawnNPC

	std::string updateOres(ore_t prometium, ore_t endurium, ore_t terbium, ore_t xenomit, ore_t prometid, ore_t duranium, ore_t promerium);

	std::string buyAmmo(lasertype_t type, size_t amount); //include 5|f|F|U and ...F|C and ...S when DB is included
	std::string buyRockets(rockettype_t type, size_t amount); // ^

															//maybe set this to userID specific
	std::string setOrePrices(); // 10,15,25,200,200,500 on default
	std::string setRocketPrices();
	std::string setAmmoPrices();

	std::string removeOpponent(id_t id);
	std::string createPortal(portalid_t portalID, portaltype_t portalType, portalid_t ptarid, pos_t x, pos_t y);
	std::string createStation(stationid_t stationID, stationtype_t stationType, stationname_t stationName, factionid_t faction, bool stationPeace, pos_t x, pos_t y);

	std::string updateSpeed(speed_t speed);
	std::string missSelf();
	std::string missSelected();
	std::string damageBubbleSelf(health_t health, shield_t shield, damage_t dmg);
	std::string damageBubbleSelected(health_t health, shield_t shield, damage_t dmg, bool updateAmmo);
	std::string displayStarSystem();
	std::string updateSettings(bool boostenwtfisthis, bool displayDamage, bool displayAllLaser, bool displayExplosions, bool displayPlayerNames, bool displayCompanyIcon, bool displayAlphaBackground, bool ignoreResources, bool ignoreBoxes, bool convertGateswtfisthis, bool convertOpponentwtfisthis, bool playSound, bool playMusic, bool displayStatus, bool displayAttackbubble, bool selectedLaser, bool selectedRocket, bool displayChat, bool displayDrones, bool showStarsystem, bool ignoreCargo, bool ignoreHostileCargo, bool autochangeAmmo, bool enableFastBuy);

	std::string showBars(health_t hp, health_t hpmax, shield_t shd, shield_t shdmax);

	std::string kill(id_t id);

};



#endif