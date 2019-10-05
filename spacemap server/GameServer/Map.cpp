#include "Map.h"

extern std::mutex database_mutex;
extern CDBGetter g_database_get;
extern std::map<map_t, CMap> g_allMaps;
const std::map<map_t, CMap> CMap::generateMapMap()
{
	std::map<map_t, CMap> m;
	int i;
	bool start, fight = false;
	factionid_t fac = 0;
	level_t lvlEn = 0;
	level_t lvlMem = 0;
	for (i = 1; i <= 29; i++)
	{
		{
			std::lock_guard<std::mutex> lock(database_mutex);
			g_database_get.changeQuery("isStarterMap", "maps", "id", i);
			start = g_database_get.getInt();
			g_database_get.changeQuery("factionId", "maps", "id", i);
			fac = g_database_get.getInt();
			g_database_get.changeQuery("levelEnemyFactionRequired", "maps", "id", i);
			lvlEn = g_database_get.getInt();
			g_database_get.changeQuery("levelOwnFactionRequired", "maps", "id", i);
			lvlMem = g_database_get.getInt();
		}

		if (i == 13 || i == 14 || i == 15 || i == 16)
			fight = true;
		else
			fight = false;

		m.emplace(std::piecewise_construct,
			std::forward_as_tuple(i),
			std::forward_as_tuple(i, fac, fight, start, lvlEn, lvlMem));
	}
	fight = false;
	i = 42;
	{
		std::lock_guard<std::mutex> lock(database_mutex);
		g_database_get.changeQuery("isStarterMap", "maps", "id", i);
		start = g_database_get.getInt();
		g_database_get.changeQuery("factionId", "maps", "id", i);
		fac = g_database_get.getInt();
		g_database_get.changeQuery("levelEnemyFactionRequired", "maps", "id", i);
		lvlEn = g_database_get.getInt();
		g_database_get.changeQuery("levelOwnFactionRequired", "maps", "id", i);
		lvlMem = g_database_get.getInt();
	}
	m.emplace(std::piecewise_construct,
		std::forward_as_tuple(42),
		std::forward_as_tuple(42, fac, false, start, lvlEn, lvlMem));

	m.emplace(std::piecewise_construct,
		std::forward_as_tuple(55),
		std::forward_as_tuple(55, 0, false, false, 1, 1));
	return m;
}

void CMap::initPortals() {
		std::string			regexdbget;
		{
			//NOW INITALIZED ON GAMESERVER START
			std::lock_guard<std::mutex> lock(database_mutex);
			g_database_get.changeQuery("Portals", "maps", "id", mapID);
			regexdbget = g_database_get.getString();
		}

		regexPortalFilter(regexdbget);

}

void CMap::regexPortalFilter(std::string regextxt)
{
	std::string extractschemata = R"("Id":([0-9]+),"x":"([0-9]+)\","y":"([0-9]+)\","newX":"([0-9]+)\",\"newY":"([0-9]+)\",\"Map\":([0-9]+))";
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
		// side note: this is not 100% accurate: level 3 can jump between x-3 <-> x-4 while level 2 can only jump x-3 <-> x-2 <-> x-4 
		// but idc thats not good i hate this idea anyway
		level_t level_own = 0, level_enemy = 0;
		factionid_t fac = 0;
		map_t goalMapID = lexical_cast<map_t>(m[6]);
		CMap& goalMap = g_allMaps.at(goalMapID); //expected to exist
		portals.emplace_back(lexical_cast<portalid_t>(m[1]),
			lexical_cast<pos_t>(m[2]), lexical_cast<pos_t>(m[3]),
			lexical_cast<pos_t>(m[4]), lexical_cast<pos_t>(m[5]),
			goalMapID, goalMap.m_pLevelRequiredMember, goalMap.m_pLevelRequiredMember, goalMap.m_pFactionID);
		regextxt = m.suffix().str();
	}

}
