#include "Map.h"

extern std::mutex database_mutex;
extern CDBGetter g_database_get;

void CMap::initPortals() {
		std::string			regexdbget;

		{
			//NOW INITALIZED ON GAMESERVER START
			//std::lock_guard<std::mutex> lock(database_mutex);
			g_database_get.changeQuery("Portals", "maps", "id", mapID);
			regexdbget = g_database_get.getString();
		}

		regexPortalFilter(regexdbget);

}
