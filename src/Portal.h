// regex [{"Id":1,"x":"2000","y":"2000","newX":"18500","newY":"11500","CMap":1} {...}]
//		Scheme: {"Id":[0-9]+,"x":"[0-9]+","y":"[0-9]+","newX":"[0-9]+","newY":"[0-9]+","CMap":"[0-9]+}
// -> see CMap for implemention
#ifndef PORTAL_H
#define PORTAL_H
#include <iostream>
#include "Defines.h"

struct CPortal {
	constexpr const static size_t	RANGE = 500;
	portalid_t						portalIDonMap;
	pos_t							x;
	pos_t							y;
	pos_t							destination_x;
	pos_t							destination_y;
	map_t							destination_mapID;
	level_t							level_for_own_company;
	level_t							level_for_enemy_company;
	factionid_t						faction;

	CPortal(const portalid_t pid,
		const pos_t x,
		const pos_t y,
		const pos_t dx,
		const pos_t dy,
		const map_t dmid,
		const level_t lfoc,
		const level_t lfec,
		const factionid_t fac) {
		portalIDonMap		= pid;
		this->x				= x;
		this->y				= y;
		destination_x		= dx;
		destination_y		= dy;
		destination_mapID	= dmid;
		level_for_own_company = lfoc;
		level_for_enemy_company = lfec;
		faction = fac;
	}
};
#endif