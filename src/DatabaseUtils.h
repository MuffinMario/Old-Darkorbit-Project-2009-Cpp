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
#include "DBUpdater.h"

namespace DBUtil {
	namespace funcs {
		username_t		getUsername(id_t id);
		factionid_t		getCompany(id_t id);
		damage_t		getDamage(id_t id);
		shipid_t		getShip(id_t  id);
		bool			isAdmin(id_t  id);
		bool			isInvisible(id_t  id);
		rank_t			getRank(id_t id);
		Position_t		getPos(id_t id);
		map_t			getMap(id_t id);
		health_t		getHP(id_t id);
		health_t		getmaxHP(id_t id);
		shield_t		getSHD(id_t id);
		shield_t		getmaxSHD(id_t id);
		speed_t			getSpeed(id_t id);
		level_t			getLevel(id_t id);

		cargo_t			getPrometium(id_t id);
		cargo_t			getEndurium(id_t id);
		cargo_t			getTerbium(id_t id);
		cargo_t			getPrometid(id_t id);
		cargo_t			getDuranium(id_t id);
		cargo_t			getPromerium(id_t id);
		cargo_t			getXenomit(id_t id);

		cargo_t			getCargo(id_t id);

		exp_t			getEXP(id_t id);
		hon_t			getHON(id_t id);
		credits_t		getCRD(id_t id);
		uri_t			getURI(id_t id);
		jackpot_t		getJP(id_t id);
		std::string		getDrones(id_t id);
		clan_t			getClanTag(id_t clan_id);
		bool			isPremium(id_t userId);
		clanid_t		getUserClanId(id_t id);
		clan_t			getUserClanTag(id_t id);

		void			setUsername(id_t id, username_t username);
		void			setCompany(id_t id, factionid_t company);
		void			setShip(id_t  id,shipid_t shipid);
		void			setAdmin(id_t  id,bool admin);
		void			setInvisible(id_t id, bool inv);
		void			setRank(id_t id,rank_t rank);
		void			setPos(id_t id,Position_t pos);
		void			setMap(id_t id,map_t map);
		void			setHP(id_t id, health_t hp);
		void			setmaxHP(id_t id, health_t hpm);
		void			setSHD(id_t id,shield_t shd);
		void			setmaxSHD(id_t id,shield_t shdm);
		void			setSpeed(id_t id,speed_t speed);
		void			setLevel(id_t id, level_t lvl);
		void			setCargo(id_t id,cargo_t cargo);
		void			setEXP(id_t id, exp_t exp);
		void			setHON(id_t id,hon_t hon);
		void			setCRD(id_t id,credits_t cred);
		void			setURI(id_t id,uri_t uri);
		void			setJP(id_t id,jackpot_t jackp);
		void			setDrones(id_t id,std::string drone_str);
		void			setUserClanId(id_t id,clanid_t clanid);

		void			removePrometium(id_t id, ore_t amount);
		void			removeEndurium(id_t id, ore_t amount);
		void			removeTerbium(id_t id, ore_t amount);
		void			removePrometid(id_t id, ore_t amount);
		void			removeDuranium(id_t id, ore_t amount);
		void			removePromerium(id_t id, ore_t amount);
		void			removeXenomit(id_t id, ore_t amount);

	}
}
#endif