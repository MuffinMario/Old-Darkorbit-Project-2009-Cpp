// regex [{"Id":1,"x":"2000","y":"2000","newX":"18500","newY":"11500","Map":1} {...}]
//		Scheme: {"Id":[0-9]+,"x":"[0-9]+","y":"[0-9]+","newX":"[0-9]+","newY":"[0-9]+","Map":"[0-9]+}
// -> see Map for implemention
#ifndef PORTAL_H
#define PORTAL_H
#include <iostream>
#include "Defines.h"

struct Portal {
	constexpr const static size_t	RANGE = 500;
	portalid_t						portalIDonMap;
	pos_t							x;
	pos_t							y;
	pos_t							destination_x;
	pos_t							destination_y;
	map_t							destination_mapID;

	Portal(const portalid_t pid, const pos_t x, const pos_t y, const pos_t dx, const pos_t dy, const map_t dmid) {
		portalIDonMap		= pid;
		this->x				= x;
		this->y				= y;
		destination_x		= dx;
		destination_y		= dy;
		destination_mapID	= dmid;
	}
};
#endif