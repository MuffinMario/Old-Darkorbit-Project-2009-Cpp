#pragma once
#include "Defines.h"

class CRocketInfo
{
	damage_t m_dmg;
	pos_t m_maxDistance;
	rockettype_t m_type;
	std::string m_name;

public:
	CRocketInfo() :
		m_dmg(0),
		m_maxDistance(0),
		m_type(0),
		m_name("undefined")
	{

	}
	CRocketInfo(std::string name, damage_t dmg, pos_t maxDistance, rockettype_t type) :
		m_name(name),
		m_dmg(dmg),
		m_maxDistance(maxDistance),
		m_type(type)
	{

	}
	damage_t getDamage() { return m_dmg; }
	pos_t getMaxDistance() { return m_maxDistance; }
	rockettype_t getType() { return m_type; }
	std::string getName() { return m_name; }

	void setDamage(damage_t dmg) { m_dmg = dmg; }
	void setMaxDistance(pos_t dist) { m_maxDistance= dist; }
	void setType(rockettype_t type) { m_type = type; }
	void setName(std::string& name) { m_name = name; }
	void setName(const char* name) { m_name = to_string(name); }
};