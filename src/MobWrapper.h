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
	struct Reward {
		credits_t credits;
		uri_t uri;
		exp_t exp;
		hon_t honor;
	};
	struct Loot {
		ore_t o1_prometium;
		ore_t o1_endurium;
		ore_t o1_terbium;
		ore_t o2_prometid;
		ore_t o2_duranium;
		ore_t o3_promerium;
		ore_t o4_xenomit;
	};
protected:

	Reward m_reward;
	Ship m_ship;
	Loot m_loot;
	username_t m_name;
	unsigned short m_aggressiveType;
	bool m_shareable = false;

public:
	void setShip(const Ship& ship) { this->m_ship = ship; }
	Ship getShip() const { return this->m_ship; }
	void setLoot(const Loot& loot) { this->m_loot = loot; }
	Loot getLoot() const { return this->m_loot; }
	void setReward(const Reward& reward) { this->m_reward = reward; }
	Reward getReward() const { return this->m_reward; }
	void setLootShareable(bool shareable) { this->m_shareable = shareable; }
	bool getIsLootShareable() const { return this->m_shareable; }
	void setName(const username_t& name) { this->m_name = name; }
	username_t getName() const { return this->m_name; }
	void setAggressiveType(unsigned short aggressiveType) { this->m_aggressiveType = aggressiveType; }
	unsigned short getAggressiveType() const { return this->m_aggressiveType; }

};

#endif