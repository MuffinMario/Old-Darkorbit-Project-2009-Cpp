#include "DamageManager.h"

std::random_device	DamageManager::rd;
std::mt19937		DamageManager::rng(rd());

damage_t DamageManager::damageRocket(id_t id, RocketType rockettype)
{
	if (missed()) {
		return 0;
	}

	else {
		damage_t baseDamage;

		switch (rockettype) {
		case RocketType::R310:
			baseDamage = 1000;
			break;
		case RocketType::PLT2026:
			baseDamage = 2000;
			break;
		case RocketType::PLT2021:
			baseDamage = 4000;
			break;
		}

		return dmgdiff(baseDamage);
	}
}

damage_t DamageManager::damageLaser(id_t id, lasertype_t ammotype, damage_t normalDamage, bool aimCpu)
{
	if (!aimCpu && missed()) { //if aimcpu is used, missed() is not going to get called, TO_DO: remember
		return 0;
	}
	else {
		damage_t baseDamage = normalDamage;
		switch (ammotype) {
		case 2:
		case 3:
		case 4:
			baseDamage *= ammotype;
		case 5:
			baseDamage *= 2;
		}
		return dmgdiff(baseDamage);
	}
}

bool DamageManager::missed()
{
	static std::uniform_int_distribution<int> chance(0, 7);
	return !chance(rng); //chance == 0
}

damage_t DamageManager::dmgdiff(damage_t damage) {
	std::uniform_int_distribution<int> damagedifference(-damage / 10, damage / 10);
	return damage + damagedifference(rng);
}
