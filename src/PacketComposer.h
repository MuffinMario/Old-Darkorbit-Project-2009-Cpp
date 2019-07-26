
#ifndef PACKETCOM_H
#define PACKETCOM_H
#include "Defines.h"
#include <string>
#include <limits>
#include <iostream>
#include <deque>
#include <cmath>
#include <chrono>
#include <mutex>
#include <boost\lexical_cast.hpp>

#include "Packeter.h"
#include "Constants.h"
#include "Defines.h"

//////////////////////////////////////
//		PacketComposer (i dont know how i could name this good)
//	will always return a string which builds up a packet
// side not: not static cuz too lazy for multiple threads (push possible race condition)
// more side not: static cuz race condition, fuk u past-me
/////////////////////////////////////
class CPacketComposer {

	//methods
	template<typename T>
	static void push(std::deque<std::string>& i, T t);
	template<typename T, typename ... O>
	static void push(std::deque<std::string>& i, T t, O... o);
	template<typename T, typename ... O>
	static std::deque<std::string> generateStringDeque(T t, O... other);

	static std::string roundFloatToString(float number, size_t decimalplace);

public:
	//ERR
	static std::string error_noHP();
	static std::string error_notLoggedIn();
	static std::string error_alreadyLoggedIn();
	static std::string error_invalidSession();

	static std::string move(id_t userID_, pos_t lx, pos_t ly, pos_t px, pos_t py ,long long ms);

	static std::string events(bool inDemizone, bool playRepanim, bool inTradezone, bool inRadiationzone, bool inJumpzone, bool repvouchs, pos_t x, pos_t y);
	static std::string events(bool inDemizone, bool playRepanim, bool inTradezone, bool inRadiationzone, bool inJumpzone, bool repvouchs);
	static std::string updateItems(level_t droneRepairCpuLevel, bool diploScanCpu, level_t jumpCpuLevel, lasertype_t rebuyAmmoCpuAmmoType, bool hasRepairBot, bool hasOreSellDrone, bool hasRtDoublerUNUSED, bool hasSmartBombCpu, bool hasInstashieldCpu, level_t mineBoostCpuLevel, level_t aimCpuLevel, bool hasArolCpu, bool hasCloakCpu);

	static std::string laserAttack(id_t from, id_t to, lasertype_t lasercolor);
	static std::string laserAbort(id_t from,id_t to);
	static std::string rocketAttack(id_t from, id_t to, rockettype_t rocketType, bool hit = true);

	static std::string init(id_t userID, username_t username, shipid_t shipID, speed_t speed, shield_t shield,
		shield_t shieldmax, health_t health, health_t healthmax, cargo_t cargospace, cargo_t cargospacemax, pos_t x, pos_t y,
		map_t mapID, factionid_t fractionid, clanid_t clanid, ammo_t battmax, rocket_t rocketsmax, weaponstate_t oState,
		bool isPremium, exp_t experience, hon_t honour, level_t level, credits_t credits, uri_t uridium,
		jackpot_t jackpot, rank_t userrank, clan_t clantag, gates_t gatesAchieved, bool useSysFont);
	static std::string loadMap(map_t mapid);
	static std::string loadMiniMap(map_t mapid);
	static std::string spawnEnemy(id_t userID, shipid_t shipID, weaponstate_t oState, clan_t clan, username_t username, pos_t x, pos_t y, factionid_t company, clanid_t clanID, rank_t rank, bool showRedSquareOnMinimap, clanstate_t clanDiplo, gates_t galaxygates, bool usebigfont = 1);
	//maybe do spawnNPC
	static std::string cloak(id_t who, bool invisible);
	static std::string sendMessage(std::string text);

	static std::string updateOres(ore_t prometium, ore_t endurium, ore_t terbium, ore_t xenomit, ore_t prometid, ore_t duranium, ore_t promerium);

	static std::string buyAmmo(lasertype_t type, size_t amount); //include 5|f|F|U and ...F|C and ...S when DB is included
	static std::string buyRockets(rockettype_t type, size_t amount); // ^

															//maybe set this to userID specific
	static std::string setOrePrices(credits_t prometium, credits_t endurium, credits_t terbium, 
		credits_t prometid, credits_t duranium, 
		credits_t promerium); // 10,15,25,200,200,500 on default
	static std::string setRocketPrices();
	static std::string setAmmoPrices();


	std::string cubikonSpawnProtegitEffect(id_t id);

	static std::string greyOpponent(id_t id, id_t from);
	static std::string ungreyOpponent(id_t id);
	static std::string removeOpponent(id_t id);
	static std::string removeOre(id_t id);
	static std::string removeLoot(id_t id);
	static std::string createPortal(portalid_t portalID, portaltype_t portalType, portalid_t ptarid, pos_t x, pos_t y);
	static std::string createStation(stationid_t stationID, stationtype_t stationType, stationname_t stationName, factionid_t faction, bool stationPeace, pos_t x, pos_t y);
	static std::string createOre(id_t id,ore_t ore, pos_t x, pos_t y, bool isBomb = false);
	static std::string createLoot(id_t id, id_t box,pos_t x, pos_t y);
/* 	type = 	CAR = cargo
			JPE = jackpot
			CRE = credits
			URI = uridium
			EP 	= experience
			HTP = hitpoints
			ROK = rockets
			BAT = ammo
			DHP = minus hp
			HON = honor
			XEN = extra energy -- not xenomit
	==== PARAMETERS ===
	CAR|promet|endur|terb|xen|prometid|duran|promer
	JPE|collected|total
	CRE|collected|total
	URI|collected|total
	EP|amount|total|currentlevel
	HTP|amount|total
	ROK|type|amount -- amount only changes CURRENTROCKETS, thx bugpoint
	BAT|type|amount -- ^
	DHP|amount|total
	HON|amount|total
	XEN
	*/
	static std::string receiveLoot(std::string lootType, std::initializer_list<long long>);
	static std::string receiveLoot(std::string lootType, std::initializer_list<float>);
	static std::string receiveLoot(std::string lootType);

	static std::string levelUp(level_t newLevel, exp_t expNeeded);

	static std::string updateSpeed(speed_t speed);
	static std::string missSelf();
	static std::string missSelected();
	static std::string damageBubbleSelf(health_t health, shield_t shield, damage_t dmg);
	static std::string damageBubbleSelected(health_t health, shield_t shield, damage_t dmg, bool updateAmmo);
	static std::string displayStarSystem();
	static std::string updateSettings(bool boostenwtfisthis, bool displayDamage, bool displayAllLaser, bool displayExplosions, bool displayPlayerNames, bool displayCompanyIcon, bool displayAlphaBackground, bool ignoreResources, bool ignoreBoxes, bool convertGateswtfisthis, bool convertOpponentwtfisthis, bool playSound, bool playMusic, bool displayStatus, bool displayAttackbubble, bool selectedLaser, bool selectedRocket, bool displayChat, bool displayDrones, bool showStarsystem, bool ignoreCargo, bool ignoreHostileCargo, bool autochangeAmmo, bool enableFastBuy);

	static std::string showBars(health_t hp, health_t hpmax, shield_t shd, shield_t shdmax);

	static std::string kill(id_t id);
	static std::string updateHP(health_t health, health_t max_health);
	static std::string updateSHD(shield_t shield, shield_t max_shield);
	static std::string updateHealth(health_t health, health_t max_health, shield_t shield, shield_t max_shield);

};



#endif