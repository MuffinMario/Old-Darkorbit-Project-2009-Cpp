#include "Portal.h"

#include "../common/coutcolor.h"
#include "../common/Defines.h"

CPortal::CPortal(const portalid_t pid, const pos_t x, const pos_t y, const pos_t dx, const pos_t dy, const map_t dmid, const level_t lfoc, const level_t lfec, const factionid_t fac) :
	portalIDonMap(pid),
	x(x),
	y(y),
	destination_x(dx),
	destination_y(dy),
	destination_mapID(dmid),
	level_for_own_company(lfoc),
	level_for_enemy_company(lfec),
	faction(fac)
{}
