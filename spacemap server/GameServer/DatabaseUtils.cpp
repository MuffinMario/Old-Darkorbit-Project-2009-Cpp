#include "DatabaseUtils.h"

extern CDBGetter g_database_get;
extern CDBUpdater g_database_update;

std::mutex database_mutex;
constexpr damage_t LF1_DMG = 40;
constexpr damage_t MP1_DMG = 60;
constexpr damage_t LF2_DMG = 100;
constexpr damage_t LF3_DMG = 150;
constexpr speed_t G3N_1010 = 2;
constexpr speed_t G3N_2010 = 3;
constexpr speed_t G3N_3210 = 4;
constexpr speed_t G3N_3310 = 5;
constexpr speed_t G3N_6900 = 7;
constexpr speed_t G3N_7900 = 10;
constexpr shield_t SG3N_A01 = 1000;
constexpr shield_t SG3N_A02 = 2000;
constexpr shield_t SG3N_A03 = 5000; // who thought of this
constexpr shield_t SG3N_B01 = 4000;
constexpr shield_t SG3N_B02 = 10000;

std::string DBUtil::funcs::getUsername(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("usuario", "cuentas","id", id);
	return g_database_get.getString();
}

factionid_t DBUtil::funcs::getCompany(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("empresa", "cuentas", "id", id);
	return g_database_get.getInt();
}
damage_t DBUtil::funcs::getDamage(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	damage_t dmg = 0;
	g_database_get.changeQuery("lf1", "cuentas", "id", id);
	dmg += g_database_get.getInt() * LF1_DMG;
	g_database_get.changeQuery("mp1", "cuentas", "id", id);
	dmg += g_database_get.getInt() * MP1_DMG;
	g_database_get.changeQuery("lf2", "cuentas", "id", id);
	dmg += g_database_get.getInt() * LF2_DMG;
	g_database_get.changeQuery("lf3", "cuentas", "id", id);
	dmg += g_database_get.getInt() * LF3_DMG;

	return dmg;
}

/*double DBUtil::funcs::getShieldAbsorption(id_t id)
{
	
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("nave", "cuentas", "id", id);
	return g_database_get.getInt();
}*/


shipid_t DBUtil::funcs::getShip(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("nave", "cuentas", "id", id);
	return g_database_get.getInt();
}
bool DBUtil::funcs::isAdmin(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("isAdmin", "cuentas", "id", id);
	return g_database_get.getBoolean();
}

bool DBUtil::funcs::isInvisible(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("Invisible", "cuentas", "id", id);
	return g_database_get.getBoolean();
}

rank_t DBUtil::funcs::getRank(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("rango", "cuentas", "id", id);
	return g_database_get.getInt();
}

Position_t DBUtil::funcs::getPos(id_t id)
{
	std::string rawstring;
	std::vector<std::string> pos_vec;
	Position_t pos;

	//here you can see the rare scoping in its nature - changing the db-query and setting a variable by a not far off unlocked mutex
	{
		std::lock_guard<std::mutex> lock(database_mutex);
		g_database_get.changeQuery("pos", "cuentas", "id", id);
		rawstring = g_database_get.getString();
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
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("mapa", "cuentas", "id", id);
	return g_database_get.getInt();
}

health_t DBUtil::funcs::getHP(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("hp", "cuentas", "id", id);
	return g_database_get.getInt();
}

health_t DBUtil::funcs::getmaxHP(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("hpMax", "cuentas", "id", id);
	return g_database_get.getInt();
}

shield_t DBUtil::funcs::getSHD(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("escudo", "cuentas", "id", id);
	return g_database_get.getInt();
}

shield_t DBUtil::funcs::getmaxSHD(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("escudoMax", "cuentas", "id", id);
	return g_database_get.getInt();
}

speed_t DBUtil::funcs::getSpeed(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("vel", "cuentas", "id", id);
	return g_database_get.getUInt();
}

level_t DBUtil::funcs::getLevel(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("nivel", "cuentas", "id", id);
	return g_database_get.getUInt();
}

cargo_t DBUtil::funcs::getPrometium(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("prometium", "cuentas", "id", id);
	return g_database_get.getUInt();
}
cargo_t DBUtil::funcs::getEndurium(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("endurium", "cuentas", "id", id);
	return g_database_get.getUInt();
}
cargo_t DBUtil::funcs::getTerbium(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("terbium", "cuentas", "id", id);
	return g_database_get.getUInt();
}

cargo_t DBUtil::funcs::getPrometid(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("prometid", "cuentas", "id", id);
	return g_database_get.getUInt();
}
cargo_t DBUtil::funcs::getDuranium(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("duranium", "cuentas", "id", id);
	return g_database_get.getUInt();
}

cargo_t DBUtil::funcs::getPromerium(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("promerium", "cuentas", "id", id);
	return g_database_get.getUInt();
}
cargo_t DBUtil::funcs::getXenomit(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("xenomit", "cuentas", "id", id);
	return g_database_get.getUInt();
}


cargo_t DBUtil::funcs::getCargo(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	cargo_t cargo = 0;
	g_database_get.changeQuery("prometium", "cuentas", "id", id);
	cargo += g_database_get.getUInt();
	g_database_get.changeQuery("endurium", "cuentas", "id", id);
	cargo += g_database_get.getUInt();
	g_database_get.changeQuery("terbium", "cuentas", "id", id);
	cargo += g_database_get.getUInt();
	g_database_get.changeQuery("prometid", "cuentas", "id", id);
	cargo += g_database_get.getUInt();
	g_database_get.changeQuery("duranium", "cuentas", "id", id);
	cargo += g_database_get.getUInt();
	g_database_get.changeQuery("promerium", "cuentas", "id", id);
	cargo += g_database_get.getUInt();

	return cargo;
}

exp_t DBUtil::funcs::getEXP(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("experiencia", "cuentas", "id", id);
	return g_database_get.getUInt64();
}

hon_t DBUtil::funcs::getHON(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("honor", "cuentas", "id", id);
	return g_database_get.getUInt64();
}

credits_t DBUtil::funcs::getCRD(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("creditos", "cuentas", "id", id);
	return g_database_get.getUInt64();
}

uri_t DBUtil::funcs::getURI(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("uridium", "cuentas", "id", id);
	return g_database_get.getUInt64();
}

jackpot_t DBUtil::funcs::getJP(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("jackpot", "cuentas", "id", id);
	jackpot_t jp = static_cast<jackpot_t>(g_database_get.getDouble());
	return jp;
}
std::string DBUtil::funcs::getDrones(id_t id) {
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("vants", "cuentas", "id", id);
	return g_database_get.getString();
}

clan_t DBUtil::funcs::getClanTag(id_t clan_id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("tagNombre", "clanes", "clanID", clan_id);
	return g_database_get.getString();
}
bool DBUtil::funcs::isPremium(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("premium", "cuentas", "id", id);
	return g_database_get.getBoolean();
}

clanid_t DBUtil::funcs::getUserClanId(id_t id)
{
	std::lock_guard<std::mutex> lock(database_mutex);
	g_database_get.changeQuery("clan", "cuentas", "id", id);
	return g_database_get.getInt();
}

clan_t DBUtil::funcs::getUserClanTag(id_t id)
{
	//both mutex locks inside functions
	clanid_t clan_id = getUserClanId(id);
	return getClanTag(clan_id);
}

//these two macros require parameter id_t to be called 'id'

//tostring: if not string but something else tostring(string) returns string
#define DEFAULT_SET_TEMPLATE_STR(row_name,table_name,id_string,param) std::string tmp_str = "'" + to_string(param) + "'";\
																		char* set_val = new char[tmp_str.length() + 1];\
																		strcpy_s(set_val, tmp_str.length() + 1, tmp_str.c_str());\
																		\
																		{\
																			std::lock_guard<std::mutex> lock(database_mutex);\
																			g_database_update.changeQuery(row_name, table_name, id_string, id, set_val);\
																			g_database_update.queryUpdate();\
																		}\
																		delete[] set_val;


#define DEFAULT_SET_TEMPLATE_NORMAL(row_name,table_name,id_string,param)std::string tmp_str = to_string(param);\
																		char* set_val = new char[tmp_str.length() + 1];\
																		strcpy_s(set_val, tmp_str.length() + 1, tmp_str.c_str());\
																		\
																		{\
																			std::lock_guard<std::mutex> lock(database_mutex);\
																			g_database_update.changeQuery(row_name, table_name, id_string, id, set_val);\
																			g_database_update.queryUpdate();\
																		}\
																		delete[] set_val;


void DBUtil::funcs::setUsername(id_t id, username_t username)
{
	DEFAULT_SET_TEMPLATE_STR("usuario", "cuentas", "id", username)
}

void DBUtil::funcs::setCompany(id_t id, factionid_t company)
{
	DEFAULT_SET_TEMPLATE_NORMAL("empresa", "cuentas", "id", company)
}

void DBUtil::funcs::setShip(id_t id, shipid_t shipid)
{
	DEFAULT_SET_TEMPLATE_NORMAL("nave", "cuentas", "id", shipid)
}
//WE NEED THIS?????????????????????????????????????????????????????????
void DBUtil::funcs::setAdmin(id_t id, bool admin)
{
	DEFAULT_SET_TEMPLATE_NORMAL("isAdmin", "cuentas", "id", admin)
}

void DBUtil::funcs::setRank(id_t id, rank_t rank)
{
	DEFAULT_SET_TEMPLATE_NORMAL("rango", "cuentas", "id", rank)
}

void DBUtil::funcs::setPos(id_t id, Position_t pos)
{
	DEFAULT_SET_TEMPLATE_STR("pos", "cuentas", "id", pos)
}

void DBUtil::funcs::setMap(id_t id, map_t map)
{
	DEFAULT_SET_TEMPLATE_NORMAL("mapa", "cuentas", "id", map);
}

void DBUtil::funcs::setHP(id_t id, health_t hp)
{
	DEFAULT_SET_TEMPLATE_NORMAL("hp", "cuentas", "id", hp)
}

void DBUtil::funcs::setmaxHP(id_t id, health_t hpm)
{
	DEFAULT_SET_TEMPLATE_NORMAL("hpMax", "cuentas", "id", hpm)
}

void DBUtil::funcs::setSHD(id_t id, shield_t shd)
{
	DEFAULT_SET_TEMPLATE_NORMAL("escudo", "cuentas", "id", shd)
}

void DBUtil::funcs::setmaxSHD(id_t id, shield_t shdm)
{
	DEFAULT_SET_TEMPLATE_NORMAL("escudoMax", "cuentas", "id", shdm)
}

void DBUtil::funcs::setSpeed(id_t id, speed_t speed)
{
	DEFAULT_SET_TEMPLATE_NORMAL("vel", "cuentas", "id", speed)
}

void DBUtil::funcs::setLevel(id_t id, level_t lvl)
{
	DEFAULT_SET_TEMPLATE_NORMAL("nivel", "cuentas", "id", lvl)
}
void DBUtil::funcs::setInvisible(id_t id, bool inv)
{
	DEFAULT_SET_TEMPLATE_NORMAL("Invisible", "cuentas", "id", inv)
}

void DBUtil::funcs::setCargo(id_t id, cargo_t cargo)
{
	DEFAULT_SET_TEMPLATE_NORMAL("carga", "cuentas", "id", cargo) //todo think how to distribute carga | cargamax
}

void DBUtil::funcs::setEXP(id_t id, exp_t exp)
{
	DEFAULT_SET_TEMPLATE_NORMAL("experiencia", "cuentas", "id", exp)
}

void DBUtil::funcs::setHON(id_t id, hon_t hon)
{
	DEFAULT_SET_TEMPLATE_NORMAL("honor", "cuentas", "id", hon)
}

void DBUtil::funcs::setCRD(id_t id, credits_t cred)
{
	DEFAULT_SET_TEMPLATE_NORMAL("creditos", "cuentas", "id", cred)
}

void DBUtil::funcs::setURI(id_t id, uri_t uri)
{
	DEFAULT_SET_TEMPLATE_NORMAL("uridium", "cuentas", "id", uri)
}

void DBUtil::funcs::setJP(id_t id, jackpot_t jackp)
{
	DEFAULT_SET_TEMPLATE_NORMAL("jackpot","cuentas","id",jackp)
}

void DBUtil::funcs::setDrones(id_t id, std::string drone_str)
{
	DEFAULT_SET_TEMPLATE_STR("vants", "cuentas", "id", drone_str)
}

void DBUtil::funcs::setUserClanId(id_t id, clanid_t clanid)
{
	DEFAULT_SET_TEMPLATE_NORMAL("clan", "cuentas", "id", clanid)
}


#define DEFAULT_REDUCE_TEMPLATE_NORMAL(row_name,table_name,id_string,param,amount)\
									std::string str = "UPDATE ";\
									str += table_name;\
									str += " SET ";\
									str += row_name;\
									str += " = ";\
									str += row_name;\
									str += " - "; \
									str += to_string(amount);\
									str += " WHERE ";\
									str += id_string;\
									str += " = ";\
									str += to_string(param);\
									g_database_update.queryUpdateRaw(str);

void DBUtil::funcs::removePrometium(id_t id, ore_t amount)
{
	DEFAULT_REDUCE_TEMPLATE_NORMAL("prometium", "cuentas", "id", id, amount);
}

void DBUtil::funcs::removeEndurium(id_t id, ore_t amount)
{
	DEFAULT_REDUCE_TEMPLATE_NORMAL("endurium", "cuentas", "id", id, amount);
}

void DBUtil::funcs::removeTerbium(id_t id, ore_t amount)
{
	DEFAULT_REDUCE_TEMPLATE_NORMAL("terbium", "cuentas", "id", id, amount);
}

void DBUtil::funcs::removePrometid(id_t id, ore_t amount)
{
	DEFAULT_REDUCE_TEMPLATE_NORMAL("prometid", "cuentas", "id", id, amount);
}

void DBUtil::funcs::removeDuranium(id_t id, ore_t amount)
{
	DEFAULT_REDUCE_TEMPLATE_NORMAL("duranium", "cuentas", "id", id, amount);
}

void DBUtil::funcs::removePromerium(id_t id, ore_t amount)
{
	DEFAULT_REDUCE_TEMPLATE_NORMAL("promerium", "cuentas", "id", id, amount);
}

void DBUtil::funcs::removeXenomit(id_t id, ore_t amount)
{
	DEFAULT_REDUCE_TEMPLATE_NORMAL("xenomit", "cuentas", "id", id, amount);
}
