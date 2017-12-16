#ifndef DamageManager_H
#define DamageManager_H
#include <boost\weak_ptr.hpp>
#include <boost\shared_ptr.hpp>

#include <random>

#include "Defines.h"
#include "Enums.h"

class DamageManager {
	static std::random_device rd;
	static std::mt19937		  rng;
public:
	damage_t damageRocket(id_t id, RocketType rockettype);
	damage_t damageLaser(id_t id, lasertype_t ammotype, damage_t normalDamage, bool aimCpu);
private:
	static bool		missed();
	static damage_t dmgdiff(damage_t damage);
};
#endif