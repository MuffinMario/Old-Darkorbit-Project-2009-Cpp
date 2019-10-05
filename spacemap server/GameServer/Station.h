#ifndef STATION_H
#define STATION_H
#include <string>
#include "../common/Defines.h"

struct CStation {
	constexpr static size_t RANGE = 1000;
	stationid_t stationIDOnMap;
	stationname_t stationName;
	pos_t x;
	pos_t y;
	factionid_t company;
	CStation(const stationid_t sid, const stationname_t& sn, const pos_t x, const pos_t y, const factionid_t c)
		:stationIDOnMap(sid), stationName(sn),
		x(x),y(y),
		company(c)
	{

	}
};
#endif