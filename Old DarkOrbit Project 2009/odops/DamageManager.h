#ifndef DamageManager_H
#define DamageManager_H
#include <boost\weak_ptr.hpp>
#include <boost\shared_ptr.hpp>

#include <iostream>
#include <random>

#include "Defines.h"
#include "Enums.h"

class CDamageManager {
public:
	damage_t damageRocket(id_t id, ERocketType rockettype);
	damage_t damageLaser(id_t id, lasertype_t ammotype, damage_t normalDamage, bool aimCpu);
	static ELaserColor laserTypeToColor(lasertype_t laserType, weaponstate_t oState);
private:
	static bool		missed();
	static damage_t dmgdiff(damage_t damage);
};
#endif