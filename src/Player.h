#ifndef PLAYER_H
#define PLAYER_H
#include <string>

/*unused
struct Player {
	pos_t		x;
	pos_t		y;
	health_t	health;
	health_t	max_health;
	shield_t	shield;
	shield_t	max_shield;
	factionid_t company;
	CMap			currentMap;
};*/
/*
std::string updateItems(level_t droneRepairCpuLevel,
bool diploScanCpu, level_t jumpCpuLevel, lasertype_t rebuyAmmoCpuAmmoType, 
bool hasRepairBot, bool hasOreSellDrone, bool hasRtDoublerUNUSED, bool hasSmartBombCpu, 
bool hasInstashieldCpu, level_t mineBoostCpuLevel, level_t aimCpuLevel, bool hasArolCpu, 
bool hasCloakCpu);

*/
struct CItems
{
	level_t m_droneRepairCpuLevel;
	bool m_hasOiploScan;
	level_t m_jumpCpuLevel;
	lasertype_t m_rebuyAmmoCpuAmmoType;
	//bool on packet
	level_t m_repairBotLevel;
	bool m_hasOreSellDrone;
	bool m_hasRtDoubler;
	bool m_hasSmartBombCpu;
	bool m_hasInstashieldCpu;
	level_t m_mineBoostCpuLevel;
	level_t m_aimCpuLevel;
	bool m_hasArolCpu;
	bool m_hasCloakCpu;
};
//YET UNUSED
struct CConfig
{
	damage_t m_basedamage;
	CItems m_items;
	size_t m_lasers;
	shield_t m_shield;
	shield_t m_maxshield;
};
class CPlayerInfo {
public:
	CItems			items;
	bool			isadmin;
	shipid_t		shipid;
	speed_t			speed;
	shield_t		shd;
	shield_t		maxshd;
	health_t		hp;
	health_t		maxhp;
	cargo_t			cargospace;
	cargo_t			cargospacemax;
	map_t			mapid;
	factionid_t		fractionid;
	clanid_t		clanid;
	ammo_t			maxbatt;
	rocket_t		maxrock;
	weaponstate_t	oState;
	bool			premium;
	damage_t		basedamage;
	exp_t			exp;
	hon_t			hon;
	level_t			lvl;
	credits_t		cred;
	uri_t			uri;
	jackpot_t		jackpot;
	rank_t			rank;
	clan_t			clantag;
	gates_t			gatesAchieved;
	bool			useSysfont;

	std::string		drones;
	std::string		username;
};
#endif