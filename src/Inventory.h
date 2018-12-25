#pragma once
#include "Defines.h"

class CExtras
{
	id_t m_cloakInvId = 0;
	bool m_hasCloak = false;
	int m_cloaks = 0;

	bool m_hasCargoExpand = false;
	
	id_t m_jumpCPUId = 0;
	bool m_hasJumpCPU = false;
	int m_jumpCpuType = 0;
	int m_jumpCpuJumps = 0;

	bool m_hasRepBot = -1;
	int m_repBotType = 0;

	bool m_hasAutoRepBot = false;

	id_t m_droneRepairCPUId = 0;
	bool m_hasDroneRepairCPU = false;
	int m_droneRepairAmounts = 0;

	bool m_hasISH = false;

	bool m_hasSMB = false;

	bool m_hasRocketTurbo = false;

	bool m_hasAROL = false;
	bool m_hasAROLActivated = false;

	bool m_hasMineTurboCPU = false;
	int m_mineTurboType = 0;

	bool m_hasAutoAmmoBuy = false;
	lasertype_t m_ammoBuyAmmoType = 0;
	bool m_buyAmmoAuto = false;

	bool m_hasAutoRocketBuy = false;
	rockettype_t m_ammoBuyRocketType = 0;
	bool m_buyRocketAuto = false;

	bool m_hasGeneratorBoostCPU = false;

	bool m_hasLaborCPU = false;
	int m_laborCPUType = 0;
	lasertype_t m_laborCPUAmmoType = 0;
	rockettype_t m_laborCPURocketType = 0;

	bool m_hasHM7 = false;

	bool m_hasDiploCPU = false;

	bool m_hasAmmoExpand = false;
	int m_ammoExpandType = 0;


};

class CConfig
{
public:
	damage_t m_damage = 0;
	shield_t m_currentShield = 0;
	shield_t m_maxShield = 0;
	speed_t m_speed = 0;
	CExtras m_extras;
};