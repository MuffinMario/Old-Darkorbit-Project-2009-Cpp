#ifndef MAP_H
#define MAP_H

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <boost\lexical_cast.hpp>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "Enums.h"
#include "Portal.h"
#include "Station.h"
#include "DBGetter.h"
#include "DatabaseUtils.h"

extern DBGetter database_get;

static std::mutex local_mutex;
class Map {
	std::vector<Portal>		portals;
	std::vector<Station>	stations;
	map_t					mapID;
	const static size_t		MAPSIZE_NORMAL_X = 21000;
	const static size_t		MAPSIZE_NORMAL_Y = 14100;
	const static size_t		MAPSIZE_BIG_X	 = 42000;
	const static size_t		MAPSIZE_BIG_Y	 = 28200;
public:
	Map(map_t mid) {
		mapID = mid;

		generateObjects();
	}

	/*mutex is not going to move anytime soon
	Map& operator=(const Map& map) { 
		portals = map.portals;
		stations = map.stations;
		mapID = map.mapID;
		return *this;
	}*/

	void generateObjects() {
		portals.clear();
		stations.clear();

		initPortals();
		initStations();
	}
	
	map_t getMapId() const {
		return mapID; 
	}
	std::vector<Station> getStations() const { 
		return stations; 
	}
	std::vector<Portal> getPortals() const { 
		return portals; 
	}

	void setMapId(map_t mapID) { 
		this->mapID = mapID; 
	}
	void setStations(std::vector<Station>& stations) { 
		this->stations = stations; 
	}
	void setPortals(std::vector<Portal>& portals) {
		this->portals = portals; 
	} 

private:
	void initPortals() {
		try {
			std::string			regexdbget;

			{
				std::lock_guard<std::mutex> lock(local_mutex);
				database_get.changeQuery("Portals", "maps", mapID);
				regexdbget = database_get.getString();
			}

			regexPortalFilter(regexdbget);
		}
		catch (sql::SQLException& ex) {
			std::cerr << "[err] Error connecting to the database: " << ex.what() << std::endl;
		}

	}

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
				stationSize - g_offsetY, FactionType::MMO);
		else if (mapID == 5)
			stations.emplace_back(1, "eicBase",
				MAPSIZE_NORMAL_X - stationSize - g_offsetX,
				stationSize - g_offsetY, FactionType::EIC);
		else if (mapID == 9)
			stations.emplace_back(1, "vruBase",
				MAPSIZE_NORMAL_X - stationSize - g_offsetX,
				MAPSIZE_NORMAL_Y - stationSize - g_offsetY, FactionType::VRU);
		else if (mapID == 20)
			stations.emplace_back(1, "mmoBase2",
				stationSize - g_offsetX,
				(MAPSIZE_NORMAL_Y - stationSize ) / 2- g_offsetY, FactionType::MMO);
		else if (mapID == 24)
			stations.emplace_back(1, "eicBase2",
				MAPSIZE_NORMAL_X / 2,
				stationSize - g_offsetY, FactionType::EIC);
		else if (mapID == 28)
			stations.emplace_back(1, "vruBase2",
				MAPSIZE_NORMAL_X - stationSize - g_offsetX,
				(MAPSIZE_NORMAL_Y - stationSize) / 2 - g_offsetY, FactionType::VRU);
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