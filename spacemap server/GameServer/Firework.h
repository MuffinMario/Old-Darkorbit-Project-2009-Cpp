#pragma once
#include "../common/Defines.h"
#include "Collectable.h"
class CFirework : public ICollectable {
public:
	enum EFireworkColor {
		RED = 0,
		BLUE,
		GREEN
	};
	enum EFireworkSize {
		S = 1,
		M,
		L
	};

private:
	id_t m_owner;
	std::array<EFireworkColor, 3> m_explosionColors;
	std::atomic<long long> m_maxAliveTime;
	const static long long m_sMAXALIVESECONDS = 600;
public:
	CFirework(id_t id, EFireworkSize size, id_t owner, std::array<EFireworkColor, 3> colors, Position_t& pos, CSession& session);
	/*
		small/medium:
		1/2 .. fwcolor[0] .. fwcolor[1]
		big:
		3+fwcolor[0] .. fwcolor[1] .. fwcolor[2]
	*/
	static id_t generateTypeID(const EFireworkSize& size, const std::array<EFireworkColor, 3> & colors);


	void spawn();
	void spawn(id_t to);
	/*
		Fireworks can't be collected, they explode on command (FWI or FWG on group).
	*/
	void collect();

	void setAliveTime(long long time) { m_maxAliveTime = time; }
	long long getAliveTime() { return m_maxAliveTime; }

	void fastTick(long long timeNow, CSession& currentSession) {}
	void secondTick(long long timeNow, CSession& currentSession) {}
};