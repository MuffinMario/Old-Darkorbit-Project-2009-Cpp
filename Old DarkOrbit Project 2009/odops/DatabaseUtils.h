#ifndef DBUTILS_H
#define DBUTILS_H


#include <utility> // pair for getpos
#include <mutex>
#include <string>
#include <vector>

#include <boost\algorithm\string.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\exception_ptr.hpp>

#include "Defines.h"
#include "Handler.h"
#include "DBGetter.h"

namespace DBUtil {
	namespace funcs {
		username_t		getUsername(id_t id);
		factionid_t		getCompany(id_t id);
		shipid_t		getShip(id_t  id);
		rank_t			getRank(id_t id);
		Position_t		getPos(id_t id);
		map_t			getMap(id_t id);
		health_t		getHP(id_t id);
		health_t		getmaxHP(id_t id);
		shield_t		getSHD(id_t id);
		shield_t		getmaxSHD(id_t id);
		speed_t			getSpeed(id_t id);
		level_t			getLevel(id_t id);
		cargo_t			getCargo(id_t id);
		exp_t			getEXP(id_t id);
		hon_t			getHON(id_t id);
		credits_t		getCRD(id_t id);
		uri_t			getURI(id_t id);
		jackpot_t		getJP(id_t id);
		std::string		getDrones(id_t id);
	}
}
#endif