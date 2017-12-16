#ifndef _MOBWRAPPER_H_
#define _MOBWRAPPER_H_
#include "Defines.h"

class CMobWrapper
{
public:
	struct Ship {
		shipid_t id;
		health_t hp;
		health_t hpmax;
		shield_t shd;
		shield_t shdmax;
		damage_t dmg;
		speed_t spd;
	};
	struct Loot {
		credits_t credits;
		uri_t uri;
		exp_t exp;
		hon_t honor;
		ore_t o1_prometium;
		ore_t o1_endurium;
		ore_t o1_terbium;
		ore_t o2_prometid;
		ore_t o2_duranium;
		ore_t o3_promerium;
		ore_t o4_xenomit;
	};
protected:

	Ship ship;
	Loot loot;
	username_t name;
	unsigned short aggressiveType;

public:
	void setShip(Ship ship) { this->ship = ship; }
	Ship getShip() { return this->ship; }
	void setLoot(Loot loot) { this->loot = loot; }
	Loot getLoot() { return this->loot; }
	void setName(username_t name) { this->name = name; }
	username_t getName() { return this->name; }
	void setAggressiveType(unsigned short aggressiveType) { this->aggressiveType = aggressiveType; }
	unsigned short getAggressiveType() { return this->aggressiveType; }

};

#endif