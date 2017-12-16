#include "DatabaseUtils.h"

extern DBGetter database_get;

//It's not really local
std::mutex my_local_mutex;

std::string DBUtil::funcs::getUsername(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("usuario", "cuentas", id);
	return database_get.getString();
}

factionid_t DBUtil::funcs::getCompany(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("empresa", "cuentas", id);
	return database_get.getInt();
}

shipid_t DBUtil::funcs::getShip(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("nave", "cuentas", id);
	return database_get.getInt();
}

rank_t DBUtil::funcs::getRank(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("rango", "cuentas", id);
	return database_get.getInt();
}

Position_t DBUtil::funcs::getPos(id_t id)
{
	std::string rawstring;
	std::vector<std::string> pos_vec;
	Position_t pos;

	//here you can see the rare scoping in its nature - changing the db-query and setting a variable by a not far off destroyed mutex
	{
		std::lock_guard<std::mutex> lock(my_local_mutex);
		database_get.changeQuery("pos", "cuentas", id);
		rawstring = database_get.getString();
	}

	boost::algorithm::split(pos_vec, rawstring, boost::is_any_of("|"));
	try {
		pos.first = boost::lexical_cast<int>(pos_vec.at(0));
		pos.second = boost::lexical_cast<int>(pos_vec.at(1));
	}
	catch (boost::bad_lexical_cast& e) {
		std::cerr << BOOST_CERR_OUTPUT(e) << std::endl;
	}
	
	return pos;
}

map_t DBUtil::funcs::getMap(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("mapa", "cuentas", id);
	return database_get.getInt();
}

health_t DBUtil::funcs::getHP(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("hp", "cuentas", id);
	return database_get.getInt();
}

health_t DBUtil::funcs::getmaxHP(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("hpMax", "cuentas", id);
	return database_get.getInt();
}

shield_t DBUtil::funcs::getSHD(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("escudo", "cuentas", id);
	return database_get.getInt();
}

shield_t DBUtil::funcs::getmaxSHD(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("escudoMax", "cuentas", id);
	return database_get.getInt();
}

speed_t DBUtil::funcs::getSpeed(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("vel", "cuentas", id);
	return database_get.getUInt();
}

level_t DBUtil::funcs::getLevel(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("nivel", "cuentas", id);
	return database_get.getUInt();
}

cargo_t DBUtil::funcs::getCargo(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("carga", "cuentas", id);
	return database_get.getUInt();
}

exp_t DBUtil::funcs::getEXP(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("experienca", "cuentas", id);
	return database_get.getUInt64();
}

hon_t DBUtil::funcs::getHON(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("honor", "cuentas", id);
	return database_get.getUInt64();
}

credits_t DBUtil::funcs::getCRD(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("creditos", "cuentas", id);
	return database_get.getUInt64();
}

uri_t DBUtil::funcs::getURI(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("uridium", "cuentas", id);
	return database_get.getUInt64();
}

jackpot_t DBUtil::funcs::getJP(id_t id)
{
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("jackpot", "cuentas", id);
	return static_cast<jackpot_t>(database_get.getDouble());
}
std::string DBUtil::funcs::getDrones(id_t id) {
	std::lock_guard<std::mutex> lock(my_local_mutex);
	database_get.changeQuery("vants", "cuentas", id);
	return database_get.getString();
}
