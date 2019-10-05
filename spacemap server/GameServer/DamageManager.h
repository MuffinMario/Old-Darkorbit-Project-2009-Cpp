#ifndef DamageManager_H
#define DamageManager_H
#include <boost\weak_ptr.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\algorithm\clamp.hpp>

#include <iostream>
#include <random>

#include "../common/Defines.h"
#include "Enums.h"

class CDamageManager {
public:
	damage_t damageRocket(id_t id, ERocketType rockettype);
	damage_t damageLaser(id_t id, lasertype_t ammotype, damage_t normalDamage, bool aimCpu);
	static ELaserColor laserTypeToColor(lasertype_t laserType, weaponstate_t oState);
	void updateHitChance(int percent0To100);
private:
	int missChance = 10; // 10% 
	bool		missed();
	static damage_t dmgdiff(damage_t damage);
};
#endif