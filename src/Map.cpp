#include "Map.h"

extern std::mutex database_mutex;
extern CDBGetter g_database_get;

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
		// side note: this is not 100% accurate: level 3 can jump between x-3 and x-4 while level 2 can only jump x-3 <-> x-2 <-> x-4 
		// but idc thats not good i hate this idea
		level_t level_own = 0, level_enemy = 0;
		factionid_t fac = 0;
		map_t goalMap = lexical_cast<map_t>(m[6]);
		{
			std::lock_guard<std::mutex> lock(database_mutex);
			g_database_get.changeQuery("levelOwnFactionRequired", "maps", "id", goalMap);
			level_own = g_database_get.getInt();
			g_database_get.changeQuery("levelEnemyFactionRequired", "maps", "id", goalMap);
			level_enemy = g_database_get.getInt();
			g_database_get.changeQuery("factionId", "maps", "id", goalMap);
			fac = g_database_get.getInt();
			//dcout << "[Map " << goalMap << "] " << (fac == 0?"None":(fac==1?"MMO":(fac==2?"EIC":(fac==3?"VRU":"???")))) << " / " << level_own << " / " << level_enemy << cendl;
		}
		portals.emplace_back(lexical_cast<portalid_t>(m[1]),
			lexical_cast<pos_t>(m[2]), lexical_cast<pos_t>(m[3]),
			lexical_cast<pos_t>(m[4]), lexical_cast<pos_t>(m[5]),
			goalMap, level_own, level_enemy, fac);
		regextxt = m.suffix().str();
	}

}
