#include "DamageManager.h"

damage_t CDamageManager::damageRocket(id_t id, ERocketType rockettype)
{
	if (missed()) {
		return 0;
	}

	else {
		damage_t baseDamage;

		switch (rockettype) {
		case ERocketType::R310:
			baseDamage = 1000;
			break;
		case ERocketType::PLT2026:
			baseDamage = 2000;
			break;
		case ERocketType::PLT2021:
			baseDamage = 4000;
			break;
		}

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
	return !random<unsigned short>(0,7); //chance == 0
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
		}
	}
	return laserColor;
}
