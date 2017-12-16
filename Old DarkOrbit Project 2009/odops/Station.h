#ifndef STATION_H
#define STATION_H
#include <string>
#include "Defines.h"

struct Station {
	const static size_t RANGE = 850;
	stationid_t stationIDonMap;
	stationname_t stationName;
	pos_t x;
	pos_t y;
	factionid_t company;
	Station(stationid_t sid, stationname_t sn, pos_t x, pos_t y, factionid_t c) {
		stationIDonMap = sid;
		stationName = sn;
		this->x = x;
		this->y = y;
		company = c;
	}
};
#endif