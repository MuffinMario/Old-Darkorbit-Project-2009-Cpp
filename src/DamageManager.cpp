#include "DamageManager.h"

#include "RocketInfo.h"
#include <map>

extern std::map<rockettype_t, CRocketInfo> g_rocketinfo;

damage_t CDamageManager::damageRocket(id_t id, ERocketType rockettype)
{
	if (missed()) {
		return 0;
	}

	else {
		damage_t baseDamage = g_rocketinfo[rockettype].getDamage();

		return dmgdiff(baseDamage);
	}
}

damage_t CDamageManager::damageLaser(id_t id, lasertype_t ammotype, damage_t normalDamage, bool aimCpu)
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
			break;
		case 5:
			baseDamage *= 2;
			break;
		}
		return dmgdiff(baseDamage);
	}
}

bool CDamageManager::missed()
{
	return random<unsigned short>(0,99)  < missChance;
}

damage_t CDamageManager::dmgdiff(damage_t damage) {
	return damage + random<damage_t>(-damage / 10, damage / 10);
}

ELaserColor CDamageManager::laserTypeToColor(lasertype_t laserType, weaponstate_t oState)
{
	ELaserColor laserColor;

	if (oState == 3) // ELITE PEW PEW 
	{
		switch (laserType)
		{
		case 1:
		case 2:
			laserColor = ELaserColor::BLUE;
			break;
		case 3:
			laserColor = ELaserColor::GREEN;
			break;
		case 4:
			laserColor = ELaserColor::WHITE;
			break;
		case 5:
			laserColor = ELaserColor::SAB;
			break;
		}
	}
	else // WEAK PEW PEW
	{
		switch (laserType)
		{
		case 1:
		case 2:
		case 3:
			laserColor = ELaserColor::RED;
			break;
		case 4:
			laserColor = ELaserColor::WHITE;
			break;
		case 5:	
			laserColor = ELaserColor::SAB;
			break;
		default:
			laserColor = ELaserColor::RED;
			break;
		}
	}
	return laserColor;
}

void CDamageManager::updateHitChance(int percent0To100)
{
	missChance = 100 - boost::algorithm::clamp(percent0To100, 0, 100);
}
