#pragma once
#include "../common/Defines.h"

class CShipInfo
{
	shipid_t m_shipId = 0;
	speed_t m_basespeed = 0;
	cargo_t m_maxbasecargo = 0;
	health_t m_maxhp = 0;
	ammo_t m_maxAmmo = 0;
	rocket_t m_maxRockets = 0;
	/* Other info: extras,generators,lasers */
public:
	CShipInfo() { }
	CShipInfo(shipid_t shipId, speed_t basespeed, cargo_t maxbasecargo,
		health_t maxhp, ammo_t maxAmmo, rocket_t maxRockets)
	{
		m_shipId = shipId;
		m_basespeed = basespeed;
		m_maxbasecargo = maxbasecargo;
		m_maxhp = maxhp;
		m_maxAmmo = maxAmmo;
		m_maxRockets = maxRockets;
	}
	shipid_t getShipId() const { return m_shipId; }
	speed_t getBaseSpeed() const { return m_basespeed; }
	cargo_t getCargo() const { return m_maxbasecargo; }
	health_t getBaseHP() const { return m_maxhp; }
	ammo_t getBaseAmmo() const { return m_maxAmmo; }
	rocket_t getBaseRockets() const { return m_maxRockets; }

	void setShipId(shipid_t shipId) { m_shipId = shipId; }
	void setBaseSpeed(speed_t speed) { m_basespeed = speed; }
	void setCargo(cargo_t carg) { m_maxbasecargo = carg; }
	void setBaseHP(health_t hp) { m_maxhp = hp; }
	void setBaseAmmo(ammo_t ammo) { m_maxAmmo = ammo; }
	void setBaseRockets(rocket_t rocket) { m_maxRockets = rocket; }
};