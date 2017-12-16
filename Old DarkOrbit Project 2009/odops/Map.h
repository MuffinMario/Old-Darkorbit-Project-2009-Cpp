#ifndef MAP_H
#define MAP_H

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <mutex>
#include <boost\lexical_cast.hpp>
#include <map>

#include "Enums.h"
#include "Portal.h"
#include "Station.h"
#include "DBGetter.h"


class CMap {
	std::vector<CPortal>		portals;
	std::vector<CStation>	stations;
	map_t					mapID;
public:
	const static size_t		MAPSIZE_NORMAL_X = 21000;
	const static size_t		MAPSIZE_NORMAL_Y = 14100;
	const static size_t		MAPSIZE_BIG_X	 = 42000;
	const static size_t		MAPSIZE_BIG_Y	 = 28200;
	CMap()
	{
		mapID = 0;

		//dont generate!
	}
	CMap(map_t mid) {
		mapID = mid;

		generateObjects();
	}

	void generateObjects() {
		portals.clear();
		stations.clear();

		initPortals();
		initStations();
	}
	
	map_t getMapId() const {
		return mapID; 
	}
	std::vector<CStation> getStations() const { 
		return stations; 
	}
	std::vector<CPortal> getPortals() const { 
		return portals; 
	}

	void setMapId(map_t mapID) { 
		this->mapID = mapID; 
	}
	void setStations(std::vector<CStation>& stations) { 
		this->stations = stations; 
	}
	void setPortals(std::vector<CPortal>& portals) {
		this->portals = portals; 
	} 

	static const std::map<map_t, CMap> generateMapMap()
	{
		std::map<map_t, CMap> m;
		for (int i = 1; i <= 29; i++)
		{
			m[i] = CMap(i);
		}
		m[42] = CMap(42);
		return m;
	}

private:
	void initPortals();

	void initStations() {
		//hard-coded
		size_t	stationSize = 1500;
		pos_t	g_offsetX	= 510; // this offset apparently is not 410 and i dont know why, see theory below in star-comment
		pos_t	g_offsetY	= 300; // see spacemap.swf DoAction30 for implementation and any other DoAction for usage of this shitty variable

		/* Why I did 510 instead of 410 (g_offsetX in swf is 410)
			we will keep the radius of 750 (1500 in total width and height, so its a square not a rectangle, as it apparently is 1600x1400 or something)
			it fits (see example 1-1, http://puu.sh/qpTFX/39536ec17d.png ugly line between minimap end and blue square base icon)
			also I've tried to make it as exact as it was back then
		*/

		if (mapID == 1)
			stations.emplace_back(1, "mmoBase", 
				stationSize - g_offsetX, 
				stationSize - g_offsetY, EFactionType::MMO);
		else if (mapID == 5)
			stations.emplace_back(1, "eicBase",
				MAPSIZE_NORMAL_X - stationSize * 1.5 - g_offsetX,
				stationSize - g_offsetY, EFactionType::EIC);
		else if (mapID == 9)
			stations.emplace_back(1, "vruBase",
				MAPSIZE_NORMAL_X - stationSize - g_offsetX,
				MAPSIZE_NORMAL_Y - stationSize - g_offsetY, EFactionType::VRU);
		else if (mapID == 20)
			stations.emplace_back(1, "mmoBase2",
				stationSize - g_offsetX,
				(MAPSIZE_NORMAL_Y - stationSize ) / 2- g_offsetY, EFactionType::MMO);
		else if (mapID == 24)
			stations.emplace_back(1, "eicBase2",
				MAPSIZE_NORMAL_X / 2,
				stationSize - g_offsetY, EFactionType::EIC);
		else if (mapID == 28)
			stations.emplace_back(1, "vruBase2",
				MAPSIZE_NORMAL_X - stationSize - g_offsetX,
				(MAPSIZE_NORMAL_Y - stationSize) / 2 - g_offsetY, EFactionType::VRU);
	}

	void regexPortalFilter(std::string regextxt) {
		std::string extractschemata		= R"("Id":([0-9]+),"x":"([0-9]+)\","y":"([0-9]+)\","newX":"([0-9]+)\",\"newY":"([0-9]+)\",\"Map\":([0-9]+))";
		std::regex	expr(extractschemata);
		std::smatch m;

		using boost::lexical_cast;
		//loops as long as the regular expression exists in the portal string of the map in the database
		//and the portal string of the map gets the suffix of the found regex
		// regex [0-9]  in "1 5 7 1" ->
		// loop 1: " 5 7 1"
		// loop 2: " 7 1"
		// ...
		while (std::regex_search(regextxt, m, expr)) {
			portals.emplace_back(lexical_cast<portalid_t>(m[1]), 
				lexical_cast<pos_t>(m[2]), lexical_cast<pos_t>(m[3]), 
				lexical_cast<pos_t>(m[4]), lexical_cast<pos_t>(m[5]), 
				lexical_cast<map_t>(m[6]));
			regextxt = m.suffix().str();
		}

	}
};
#endif