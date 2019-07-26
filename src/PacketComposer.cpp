#include "PacketComposer.h"
using namespace Constants::Send;

std::string CPacketComposer::roundFloatToString(float number, size_t decimalplace) {
	//i'm going to make this method due to the wrong rounding in the client and the jackpot value being shown in a wrong way (500.20000012 jackpot dollar instead of 500.20)
	size_t sizeind = decimalplace?decimalplace:decimalplace - 1; //if there is no decimal place we will remove the . ("500." -> "500") 
	if (number < 10)// 4.00 -> 4 
		sizeind += 2;
	else if (number < 100) // 94.30 -> 5
		sizeind += 3;
	else if (number < 1000) // 250.45 -> 6
		sizeind += 4;
	else if (number < 10000) //1200.00 -> 7
		sizeind += 5;
	else // 10000.00
		sizeind += 6;	
	return to_string(number).substr(0, sizeind);
}

template<typename T>
inline void CPacketComposer::push(std::deque<std::string>& i, T t) {
	i.push_back(to_string(t));
}

template<typename T, typename ... O>
inline void CPacketComposer::push(std::deque<std::string>& i, T t, O... o) {
	i.push_back(to_string(t));
	push(i,o...);
}
template <typename T, typename ... O>
inline std::deque<std::string> CPacketComposer::generateStringDeque(T t, O... other) {
	std::deque<std::string> i;
	push(i,t, other...);
	return i;
}
std::string CPacketComposer::error_noHP()
{
	CPacketer packet(ERR);
	packet.push("1");
	return packet;
}

std::string CPacketComposer::error_notLoggedIn()
{
	CPacketer packet(ERR);
	packet.push("2");
	return packet;
}

std::string CPacketComposer::error_alreadyLoggedIn()
{
	CPacketer packet(ERR);
	packet.push("3");
	return packet;
}

std::string CPacketComposer::error_invalidSession()
{
	CPacketer packet(ERR);
	packet.push("4");
	return packet;
}

std::string CPacketComposer::move(id_t userID_, pos_t lx, pos_t ly, pos_t px, pos_t py, long long ms)
{
	CPacketer packet(ALIENMOVE);
	std::deque<std::string> information = generateStringDeque(userID_, px, py);
	information.push_back(to_string(ms));
	for (auto& str : information)
		packet.push(str);
	return packet;
}

std::string CPacketComposer::events(bool inDemizone, bool playRepanim, bool inTradezone, bool inRadiationzone, bool inJumpzone, bool repvouchs, pos_t x, pos_t y)
{
	CPacketer packet(EVENTS);
	std::deque<std::string> information = generateStringDeque(
		to_string(x), to_string(y),
		inDemizone, playRepanim, inTradezone, inRadiationzone, inJumpzone, repvouchs);

	return packet.pushAndGet(information);
}
std::string CPacketComposer::events(bool inDemizone, bool playRepanim, bool inTradezone, bool inRadiationzone, bool inJumpzone, bool repvouchs)
{
	CPacketer packet(EVENTS);
	std::deque<std::string> information = generateStringDeque(
		"NaN", "NaN", // x|y
		inDemizone, playRepanim, inTradezone, inRadiationzone, inJumpzone, repvouchs);

	return packet.pushAndGet(information);
}

std::string CPacketComposer::updateItems(level_t droneRepairCpuLevel, bool diploScanCpu, level_t jumpCpuLevel, lasertype_t rebuyAmmoCpuAmmoType, bool hasRepairBot, bool hasOreSellDrone, bool hasRtDoublerUNUSED, bool hasSmartBombCpu, bool hasInstashieldCpu, level_t mineBoostCpuLevel, level_t aimCpuLevel, bool hasArolCpu, bool hasCloakCpu)
{
	CPacketer packet(SETTINGS_ITEMS);
	packet.push(generateStringDeque(droneRepairCpuLevel, diploScanCpu, jumpCpuLevel, rebuyAmmoCpuAmmoType, hasRepairBot, hasOreSellDrone, hasRtDoublerUNUSED, hasSmartBombCpu, hasInstashieldCpu, mineBoostCpuLevel, aimCpuLevel, hasArolCpu, hasCloakCpu));
	return packet;
}


std::string CPacketComposer::laserAttack(id_t from, id_t to, lasertype_t lasercolor)
{
	CPacketer packet(LASERSHOOT);
	packet.push(generateStringDeque(from, to, lasercolor));
	return packet;
}
std::string CPacketComposer::laserAbort(id_t from,id_t to)
{
	CPacketer packet(ABORTATTACK);
	packet.push(generateStringDeque(from, to));
	return packet;
}

std::string CPacketComposer::rocketAttack(id_t from, id_t to, rockettype_t rocketType, bool hit)
{
	CPacketer packet(ROCKETSHOOT);
	std::deque<std::string> information = generateStringDeque(from, to, hit,rocketType);

	return packet.pushAndGet(information);
}

std::string CPacketComposer::init(id_t userID, username_t username, shipid_t shipID, speed_t speed, shield_t shield, 
	shield_t shieldmax, health_t health, health_t healthmax, cargo_t cargospace, cargo_t cargospacemax, pos_t x, pos_t y, 
	map_t mapID, factionid_t fractionid, clanid_t clanid, ammo_t battmax, rocket_t rocketsmax, weaponstate_t oState, 
	bool isPremium, exp_t experience, hon_t honour, level_t level, credits_t credits, uri_t uridium, 
	jackpot_t jackpot, rank_t userrank, clan_t clantag, gates_t gatesAchieved, bool useSysFont)
{
	//this method takes between 29000 (smallest) to 49000 (highest) nanoseconds which is pretty long but i mean its only rarely used
	CPacketer packet(INITALIZE);
	std::deque<std::string> information = generateStringDeque(userID, username, shipID, speed, shield, shieldmax, health, healthmax, cargospace, cargospacemax, x, y, mapID, fractionid, clanid, battmax, rocketsmax,
		oState, isPremium, experience, honour, level, credits, uridium, roundFloatToString(jackpot,2), userrank, clantag, gatesAchieved, useSysFont);


	return packet.pushAndGet(information);
}
std::string CPacketComposer::loadMap(map_t mapid) {
	CPacketer packet(CHANGEMAP_I);
	return packet.pushAndGet(to_string(mapid));
}
std::string CPacketComposer::loadMiniMap(map_t mapid) {
	CPacketer packet(CHANGEMAP_M);
	return packet.pushAndGet(to_string(mapid));
}
std::string CPacketComposer::spawnEnemy(id_t userID_, shipid_t shipID, weaponstate_t oState, clan_t clan, username_t username, pos_t x, pos_t y, 
	factionid_t company, clanid_t clanID, rank_t rank, bool showRedSquareOnMinimap, clanstate_t clanDiplo, gates_t galaxygates, bool usebigfont)
{
	CPacketer packet(SPAWNOPPONENT);
	std::deque<std::string> information = generateStringDeque(userID_, shipID, oState, clan, username, x, y, company, clanID, rank, showRedSquareOnMinimap, clanDiplo, galaxygates, usebigfont);


	return packet.pushAndGet(information);
}

std::string CPacketComposer::cloak(id_t who, bool invisible)
{
	CPacketer packet(MAKEINVISIBLE);

	return packet.pushAndGet({
			to_string(who),
			to_string(invisible)
		}
	);
}

std::string CPacketComposer::sendMessage(std::string text)
{
	CPacketer packet(SENDMESSAGE);
	packet.push(text);
	return packet;

}

std::string CPacketComposer::updateOres(ore_t prometium, ore_t endurium, ore_t terbium, ore_t xenomit, ore_t prometid, ore_t duranium, ore_t promerium)
{
	CPacketer packet(UPDATEORES);
	std::deque<std::string> information = generateStringDeque(prometium, endurium, terbium, xenomit, prometid, duranium, promerium);
	return packet.pushAndGet(information);
}

std::string CPacketComposer::buyAmmo(lasertype_t lasertype, size_t amount)
{
	//5|0|b|type|amount
	CPacketer packet(BUY);
	packet.push("o");
	packet.push("b");
	return packet.pushAndGet(generateStringDeque(lasertype, amount));
}

std::string CPacketComposer::buyRockets(rockettype_t rockettype, size_t amount)
{
	CPacketer packet(BUY);
	packet.push("o");
	packet.push("r");
	return packet.pushAndGet(generateStringDeque(rockettype, amount));
}

std::string CPacketComposer::setOrePrices(credits_t prometium, credits_t endurium, credits_t terbium,
	credits_t prometid, credits_t duranium,
	credits_t promerium)
{
	CPacketer packet(STATION_SETPRICES);
	return packet.pushAndGet(generateStringDeque(prometium, endurium, terbium, prometid, duranium, promerium));
}

std::string CPacketComposer::setRocketPrices()
{
	return std::string();
}

std::string CPacketComposer::setAmmoPrices()
{
	return std::string();
}

std::string CPacketComposer::cubikonSpawnProtegitEffect(id_t id)
{
	CPacketer packet(SPAWNATDOOR);

	return packet.pushAndGet(to_string(id));
}
std::string CPacketComposer::greyOpponent(id_t id,id_t from)
{
	CPacketer packet(GREYENEMY);

	return packet.pushAndGet({ to_string(id),to_string(from) });
}

std::string CPacketComposer::ungreyOpponent(id_t id)
{
	CPacketer packet(UNGREYENEMY);

	return packet.pushAndGet(to_string(id));
}

std::string CPacketComposer::removeOpponent(id_t id)
{
	CPacketer packet(REMOVEOPPONENT);
	packet.push(to_string(id));
	return packet;
}

std::string CPacketComposer::createPortal(portalid_t portalID, portaltype_t portalType, portalid_t ptarid, pos_t x, pos_t y)
{
	CPacketer packet(SPAWNPORTAL);
	std::deque<std::string> information{
		to_string(portalID),
		to_string(portalType),
		to_string(ptarid), //UNUSED IN CLIENT (deprecated?)
		to_string(x),
		to_string(y)
	};
	
	for (auto& it : information) {
		packet.push(it);
	}
	return packet;
}

std::string CPacketComposer::createStation(stationid_t stationID, stationtype_t stationType, stationname_t stationName, factionid_t faction, bool stationPeace, pos_t x, pos_t y)
{
	CPacketer packet(SPAWNSTATION);
	std::deque<std::string> information = generateStringDeque(stationID, stationType, stationName, faction, stationPeace, x, y);
	packet.push(information);
	return packet;
}

std::string CPacketComposer::createOre(id_t id, ore_t ore, pos_t x, pos_t y, bool isBomb)
{
	CPacketer packet(SPAWNORE);
	packet.push(generateStringDeque(id, ore, x, y, isBomb));
	return packet;
}

std::string CPacketComposer::createLoot(id_t id, id_t box, pos_t x, pos_t y)
{
	CPacketer packet(SPAWNBOX);
	packet.push(generateStringDeque(id, box, x, y));
	return packet;
}
std::string CPacketComposer::receiveLoot(std::string lootType, std::initializer_list<long long> parameters)
{
	CPacketer packet(RECEIVELOOT);
	packet.push(lootType);
	for (auto& item : parameters)
	{
		packet.push(to_string(item));
	}
	return packet;
}
std::string CPacketComposer::receiveLoot(std::string lootType, std::initializer_list<float> parameters)
{
	CPacketer packet(RECEIVELOOT);
	packet.push(lootType);
	for (auto& item : parameters)
	{
		packet.push(to_string(item));
	}
	return packet;
}
std::string CPacketComposer::receiveLoot(std::string lootType)
{
	CPacketer packet(RECEIVELOOT);
	packet.push(lootType); 
	return packet;
}
std::string CPacketComposer::levelUp(level_t newLevel, exp_t expNeeded)
{
	CPacketer packet(LEVELUP);
	packet.push(generateStringDeque(newLevel,expNeeded));
	return packet;

}
std::string CPacketComposer::removeOre(id_t id)
{
	CPacketer packet(REMOVEORE);
	packet.push(generateStringDeque(id));
	return packet;
}

std::string CPacketComposer::removeLoot(id_t id)
{
	CPacketer packet(REMOVEBOX);
	packet.push(generateStringDeque(id));
	return packet;
}

std::string CPacketComposer::updateSpeed(speed_t speed)
{
	CPacketer packet(UPDATESPEED);
	packet.push(to_string(speed));
	return packet;
}

std::string CPacketComposer::missSelf()
{
	CPacketer packet(RECEIVEMISS);
	return packet;
}
std::string CPacketComposer::missSelected()
{
	CPacketer packet(MAKEMISS);
	return packet;
}
std::string CPacketComposer::damageBubbleSelf(health_t health, shield_t shield, damage_t dmg)
{
	CPacketer packet(RECEIVEDAMAGE);

	if (dmg <= 0) {
		return missSelf();
	}
	packet.push(to_string(health));
	packet.push(to_string(shield));
	packet.push("0"); // [4] is not used
	packet.push(to_string(dmg));

	return packet;
}
std::string CPacketComposer::damageBubbleSelected(health_t health, shield_t shield, damage_t dmg, bool updateAmmo)
{
	CPacketer packet(MAKEDAMAGE);

	if (dmg <= 0) {
		return missSelected();
	}

	if (updateAmmo) {
		packet.push("L");
	}
	else {
		packet.push("NaN");
	}
	packet.push(to_string(health));
	packet.push(to_string(shield));
	packet.push(to_string(dmg));

	return packet;
}

std::string CPacketComposer::displayStarSystem()
{
	return CPacketer(DISPLAYSTARSYSTEM);
}

std::string CPacketComposer::updateSettings(bool boostenwtfisthis, bool displayDamage, bool displayAllLaser, bool displayExplosions, bool displayPlayerNames, bool displayCompanyIcon, bool displayAlphaBackground, bool ignoreResources, bool ignoreBoxes, bool convertGateswtfisthis, bool convertOpponentwtfisthis, bool playSound, bool playMusic, bool displayStatus, bool displayAttackbubble, bool selectedLaser, bool selectedRocket, bool displayChat, bool displayDrones, bool showStarsystem, bool ignoreCargo, bool ignoreHostileCargo, bool autochangeAmmo, bool enableFastBuy)
{
	return std::string();
}

std::string CPacketComposer::showBars(health_t hp, health_t hpmax, shield_t shd, shield_t shdmax) {
	CPacketer packet(SHOWHPANDSHDOPPONENT);
	packet.push("0");
	packet.push("0");
	packet.push(generateStringDeque(shd, shdmax, hp, hpmax));
	return packet;
}

std::string CPacketComposer::kill(id_t id)
{
	CPacketer packet(KILLUSER);
	packet.push(to_string(id));
	return packet;
}

std::string CPacketComposer::updateHP(health_t health,health_t max_health)
{
	CPacketer packet(UPDATEHP);
	packet.push(generateStringDeque(health,max_health));
	return packet;
}

std::string CPacketComposer::updateSHD(shield_t shield,shield_t max_shield)
{
	CPacketer packet(UPDATESHD);
	packet.push(generateStringDeque(shield, max_shield));
	return packet;
}

std::string CPacketComposer::updateHealth(health_t health, health_t max_health, shield_t shield, shield_t max_shield)
{
	CPacketer packet(UPDATEHEALTH);
	packet.push(generateStringDeque(health, max_health, shield, max_shield));
	return packet;
}
