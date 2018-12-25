#ifndef STATION_H
#define STATION_H
#include <string>
#include "Defines.h"

struct CStation {
	const static size_t RANGE = 850;
	stationid_t stationIDOnMap;
	stationname_t stationName;
	pos_t x;
	pos_t y;
	factionid_t company;
	CStation(stationid_t sid, const stationname_t& sn, pos_t x, pos_t y, factionid_t c)
		:stationIDOnMap(sid), stationName(sn),
		x(x),y(y),
		company(c)
	{

	}
};
#endif