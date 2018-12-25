#ifndef ORE_H
#define ORE_H

#include <iostream>

#include "Collectable.h"
#include "Defines.h"
#include "FileWriter.h"
#include "DBUpdater.h"


class COre : public ICollectable
{
	id_t m_oretype = 0;
	//ore_t m_oreAmount = 1; // always 1 ore ew why more

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
};
#endif