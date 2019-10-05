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

#include "../common/coutcolor.h"
#include "../common/Defines.h"

class CMap {
	std::vector<CPortal>		portals;
	std::vector<CStation>	stations;

	map_t					mapID;
	size_t					m_width = 0;
	size_t					m_height = 0;

public:

	bool					m_pStarterMap = 0;
	bool					m_pFightMap = false;
	factionid_t				m_pFactionID = 0;
	level_t					m_pLevelRequiredEnemy = 0;
	level_t					m_pLevelRequiredMember = 0;

	const static size_t		MAPSIZE_NORMAL_X = 21000;
	const static size_t		MAPSIZE_NORMAL_Y = 14100;
	const static size_t		MAPSIZE_BIG_X	 = 42000;
	const static size_t		MAPSIZE_BIG_Y	 = 28200;
	const static dist_t		RADIATIONZONE_DISTANCE_WEAK = 0;
	const static dist_t		RADIATIONZONE_DISTANCE_MEDIUM = 2500;
	const static dist_t		RADIATIONZONE_DISTANCE_STRONG = 7000;
	explicit CMap() : CMap(0,0,false,false,1,1) {}
	CMap(const CMap&) = default;
	CMap(CMap&&) = default;
	CMap& operator=(const CMap&) = default;
	~CMap() = default;
	explicit CMap(map_t mid,factionid_t factionID,bool fightMap,bool starterMap,level_t levelRequiredEnemy, level_t levelRequiredMember) :

		m_pFightMap(fightMap),
		m_pFactionID(factionID),
		m_pStarterMap(starterMap),
		m_pLevelRequiredEnemy(levelRequiredEnemy),
		m_pLevelRequiredMember(levelRequiredMember)
	{
		setMapId(mid);
	}

	void generateObjects() {
		portals.clear();
		stations.clear();

		initPortals();
		initStations();
	}
	size_t getWidth() const { return m_width; }
	size_t getHeight() const { return m_height; }
	map_t getMapId() const {
		return mapID; 
	}
	std::vector<CStation>& getStations() { 
		return stations; 
	}
	std::vector<CPortal>& getPortals() { 
		return portals; 
	}

	void setMapId(map_t mapID) { 
		this->mapID = mapID;
		if (mapID == 16 || mapID == 29)
		{
			m_width = MAPSIZE_BIG_X;
			m_height = MAPSIZE_BIG_Y;
		}
		else
		{
			m_width = MAPSIZE_NORMAL_X;
			m_height = MAPSIZE_NORMAL_Y;
		}
	}
	void setStations(std::vector<CStation>& stations) { 
		this->stations = stations; 
	}
	void setPortals(std::vector<CPortal>& portals) {
		this->portals = portals; 
	} 

	static const std::map<map_t, CMap> generateMapMap();


private:
	void initPortals();

	void initStations() {
		//hard-coded
		size_t	stationSize = 1500;
		size_t	g_offsetX	= 510; // this offset apparently is not 410 and i dont know why, see theory below in star-comment
		size_t	g_offsetY	= 300; // see spacemap.swf DoAction30 for implementation and any other DoAction for usage of this shitty variable

		/* Why I did 510 instead of 410 (g_offsetX in swf is 410)
			we will keep the radius of 750 (1500 in total width and height, so its a square not a rectangle, as it apparently is 1600x1400 or something)
			it fits (see example 1-1, http://puu.sh/qpTFX/39536ec17d.png ugly line between minimap end and blue square base icon)
			also I've tried to make it as exact as it was back then
		*/
		stations.clear();
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
				(MAPSIZE_NORMAL_Y - stationSize) / 2 - g_offsetY, EFactionType::MMO);
		else if (mapID == 24)
			stations.emplace_back(1, "eicBase2",
				MAPSIZE_NORMAL_X / 2,
				stationSize - g_offsetY, EFactionType::EIC);
		else if (mapID == 28)
			stations.emplace_back(1, "vruBase2",
				MAPSIZE_NORMAL_X - stationSize - g_offsetX,
				(MAPSIZE_NORMAL_Y - stationSize) / 2 - g_offsetY, EFactionType::VRU);
	}

	void regexPortalFilter(std::string regextxt);
};
#endif