#ifndef PLAYER_H
#define PLAYER_H
#include <string>

#include "Map.h"

class Map;
//unused
struct Player {
	pos_t		x;
	pos_t		y;
	health_t	health;
	health_t	max_health;
	shield_t	shield;
	shield_t	max_shield;
	factionid_t company;
	Map			currentMap;
};


class PlayerInfo {
public:
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