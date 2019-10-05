#ifndef ORE_H
#define ORE_H

#include <iostream>

#include "Collectable.h"
#include "../common/Defines.h"
#include "../common/FileWriter.h"
#include "DBUpdater.h"


class COre : public ICollectable
{
	void rewardPlayer(id_t playerid);
public:
	enum class EOreType : id_t {
		ORE_PROMETIUM = 1,
		ORE_ENDURIUM,
		ORE_TERBIUM
		/*,
		ORE_MINE = 10, // dumb
		ORE_SMARTBOMB = 20 // dumber, dont even think about using this. spawns without offset to its size*/
	};
	COre(id_t oretype, id_t id, pos_t x, pos_t y, std::reference_wrapper<CSession> session);

	void spawn(id_t to) override;
	void spawn() override;
	void collect(id_t playerid) override;

	void fastTick(long long timeNow, CSession& currentSession) {}
	void secondTick(long long timeNow, CSession& currentSession) {}
};
#endif