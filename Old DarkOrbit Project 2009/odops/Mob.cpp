#include "Mob.h"
#include "SessionsManager.h"
extern CDBGetter g_database_get;
extern std::map<id_t,std::mutex> g_emutex;
CMob::CMob(id_t mobid, CMobWrapper wrapper, Position_t pos, CSessionsManager * session) 
	: CMob(mobid, wrapper.getShip(), wrapper.getName(), wrapper.getLoot(), wrapper.getAggressiveType(), pos, session)
{
}
CMob::CMob(id_t mobid, Ship shipinfo, username_t name, Loot loot, unsigned short aggrType, Position_t pos, CSessionsManager * session)
	: mm(pos, shipinfo.spd), session(session)
{
	this->id = mobid;
	this->ship = shipinfo;
	this->name = name;
	this->loot = loot;
	this->aggressiveType = aggrType;
	if (session != nullptr)
	{
		std::string init_str = pm.spawnEnemy(id, ship.id, 1, "", name, pos.first, pos.second, 0, 0, 0, false, 0, 0, false);
		session->sendEveryone(init_str);
		move(pos.first, pos.second); //fix client bug
	}
}

void CMob::move(pos_t x, pos_t y)
{
	//dcout << "Move id " << id << cendl;
	CPacketComposer pm; //local for testing
	mm.get_current_position();
	mm.move(x, y);
	long long destinationTime = mm.get_time_for_destination();
	std::string move_str = pm.move(id, mm.get_current_position_x(), mm.get_current_position_y(), x, y, destinationTime);
	session->sendEveryone(move_str);
}

void CMob::attack(id_t uid)
{
	selectedId = uid;
	std::string attack_str = pm.laserAttack(id, selectedId, 1);
	session->sendEveryone(attack_str);
}

inline void CMob::abort()
{
	std::string abort = pm.laserAbort(id, selectedId);
	session->sendEveryone(abort);
}

void CMob::spawn(id_t user_id)
{
	handlePtrIt h = session->getHandler(user_id);
	Position_t pos = mm.get_current_position();
	Position_t plan = mm.get_planned_position();
	std::string init_str = pm.spawnEnemy(id, ship.id, 1, "", name, pos.first, pos.second, 0, 0, 0, false, 0, 0, false);
	long long tfd = mm.get_time_for_destination();
	std::string move_str = pm.move(id, pos.first, pos.second, plan.first, plan.second, tfd);
	h.operator->()->second->sendPacket(init_str);
	h.operator->()->second->sendPacket(move_str);

}

void CMob::generateRandomWaitingTime(long long min, long long max)
{
	long long generated = random<long long>(min,max);
	waitForNextMoveTime = getTimeNow() + generated + mm.get_time_for_destination();
}

inline void CMob::generateRandomWaitingTime(long long max)
{
	generateRandomWaitingTime(0, max);
}


id_t CMob::getId() { return id; }

Position_t CMob::getPosition() { return mm.get_current_position(); }

void CMob::setSession(CSessionsManager * session)
{
	if (this->session != nullptr) dcout << "WARNING ERROR IN CODE! MOBID ALREADY IN A SESSION AND GETS PUT INTO ANOTHER(or the same) SESSION! TO FIX CALL 9-1-1" << cendl;
	this->session = session;
}

damage_t CMob::receiveDamagePure(damage_t dmg)
{
	constexpr const double SHIELDPERCENTAGE = 0.5;
	/* Damage split between shield and HP */
	damage_t shield_damage = dmg * SHIELDPERCENTAGE;
	damage_t health_damage = dmg - shield_damage; //fight me, if you want to have a (1 - SHIELDPERCENTAGE) 

												  /* HP / SHD after hit */
	health_t health_post_damage = ship.hp - health_damage;
	shield_t shield_post_damage = ship.shd - shield_damage;

	if (shield_post_damage < 0) {
		health_post_damage += shield_post_damage; //s_p_d is negative so we add that to our health 
		ship.shd = 0;
	}
	else {
		ship.shd = shield_post_damage;
	}

	if (health_post_damage <= 0) {
		ship.hp = 0;
		die();
		return -dmg; //ded
	}
	else {
		ship.hp = health_post_damage;
		return dmg; //no ded
	}
}

damage_t CMob::receiveDamageHP(damage_t dmg)
{
	damage_t realdmg = dmg;
	health_t hp = ship.hp - dmg;

	// to ensure no overflow happens
	if (hp <= 0) {
		realdmg += hp; // 5000 DMG 4000 HP -> -1000 thus 5000 + (-1000) = 4000
		ship.hp = 0;
		die();
	}
	else
	{
		ship.hp = hp;
	}

	return realdmg;
}
void CMob::die()
{

	g_emutex[id].lock();
	session->sendEveryone(pm.kill(id));
	session->sendEveryone(pm.removeOpponent(id));
	//maybe in destructor?
	session->removeMob(id);
	g_emutex[id].unlock();
}

damage_t CMob::receiveDamageSHD(damage_t dmg)
{
	damage_t realdmg = dmg;
	shield_t shd = ship.shd- dmg;

	// to ensure no overflow happens
	if (shd <= 0) {
		realdmg += shd; // 5000 DMG 4000 HP -> -1000 thus 5000 + (-1000) = 4000
		ship.shd = 0;
	}
	else
	{
		ship.shd = shd;
	}

	return realdmg;
}

CMobWrapper CMob::getMobInfoFromDB(shipid_t alienship_id)
{
	CMobWrapper mob;
	CMob::Ship ship;
	CMob::Loot loot;
	std::string name;
	unsigned short aggressiveType;

	ship.id = alienship_id;

	g_database_get.changeQuery("nombre", "npcs", "gfx", alienship_id);
	name = g_database_get.getString();

	g_database_get.changeQuery("pv", "npcs", "gfx", alienship_id);
	ship.hpmax = g_database_get.getInt();
	ship.hp = ship.hpmax;
	g_database_get.changeQuery("esc", "npcs", "gfx", alienship_id);
	ship.shdmax = g_database_get.getInt();
	ship.shd = ship.shdmax;

	//loot
	g_database_get.changeQuery("exp", "npcs", "gfx", alienship_id);
	loot.exp = g_database_get.getUInt64();
	g_database_get.changeQuery("hon", "npcs", "gfx", alienship_id);
	loot.honor = g_database_get.getUInt64();
	g_database_get.changeQuery("cre", "npcs", "gfx", alienship_id);
	loot.credits = g_database_get.getUInt64();
	g_database_get.changeQuery("uri", "npcs", "gfx", alienship_id);
	loot.uri = g_database_get.getUInt64();


	// ores (7 total) uint to ushort 
	g_database_get.changeQuery("prometium", "npcs", "gfx", alienship_id);
	loot.o1_prometium = g_database_get.getUInt();
	g_database_get.changeQuery("endurium", "npcs", "gfx", alienship_id);
	loot.o1_endurium = g_database_get.getUInt();
	g_database_get.changeQuery("terbium", "npcs", "gfx", alienship_id);
	loot.o1_terbium = g_database_get.getUInt();
	g_database_get.changeQuery("prometid", "npcs", "gfx", alienship_id);
	loot.o2_prometid = g_database_get.getUInt();
	g_database_get.changeQuery("duranium", "npcs", "gfx", alienship_id);
	loot.o2_duranium = g_database_get.getUInt();
	g_database_get.changeQuery("promerium", "npcs", "gfx", alienship_id);
	loot.o3_promerium = g_database_get.getUInt();

	g_database_get.changeQuery("xenomit", "npcs", "gfx", alienship_id);
	loot.o4_xenomit = g_database_get.getUInt();

	g_database_get.changeQuery("IA", "npcs", "gfx", alienship_id);
	//IA -> attack type? (0 and 2 dont seem to differ so why do two)
	aggressiveType = g_database_get.getUInt();

	g_database_get.changeQuery("speed", "npcs", "gfx", alienship_id);
	ship.spd = g_database_get.getUInt();
	g_database_get.changeQuery("dmg", "npcs", "gfx", alienship_id);
	ship.dmg = g_database_get.getInt();

	mob.setAggressiveType(aggressiveType);
	mob.setLoot(loot);
	mob.setName(name);
	mob.setShip(ship);
	return mob;
}

std::vector<std::shared_ptr<CMob>> CMob::generateMobsForMap(map_t mapid)
{
	std::vector<std::shared_ptr<CMob>> mobvec; // return value

	//TODO CONSTANT AND GLOBALLY ACCESSIBLE mobInformation
	std::map<shipid_t, CMobWrapper> mobInformation;
	//db npcs
	mobInformation[2] = getMobInfoFromDB(2);
	mobInformation[4] = getMobInfoFromDB(4);
	for (shipid_t id = 71; id < 82; id++)
	{
		mobInformation[id] = getMobInfoFromDB(id);
	}
	//db map
	g_database_get.changeQuery("NPCS", "maps", "id", mapid);

	//regex
	std::string npcfull = g_database_get.getString();
	std::string regexscheme = R"("npcId":([0-9]+),"Count":([0-9]+))"; //easier than portals ;) only 2 vars
	std::regex reg(regexscheme);
	std::smatch m;

	struct _npcwrap { shipid_t shipid; size_t c; };
	std::vector<_npcwrap> variants;
	while (std::regex_search(npcfull, m, reg))
	{
		variants.push_back(_npcwrap{ boost::lexical_cast<shipid_t>(m[1]), boost::lexical_cast<size_t>(m[2]) });
		npcfull = m.suffix().str();
	}

	int j = 0;
	//now we have 50x id 3 and 25x id 71 for example
	for (auto& wr : variants)
	{
		for (size_t i = 0; i < wr.c; i++)
		{

			if (mapid != 16 && mapid != 29)
				mobvec.push_back(std::make_shared<CMob>(BEGIN_MOB_IDS+j, mobInformation[wr.shipid], Position_t(random<pos_t>(CMap::MAPSIZE_NORMAL_X), random<pos_t>(CMap::MAPSIZE_NORMAL_Y)), nullptr));
			else
				mobvec.push_back(std::make_shared<CMob>(BEGIN_MOB_IDS+j, mobInformation[wr.shipid], Position_t(random<pos_t>(CMap::MAPSIZE_BIG_X), random<pos_t>(CMap::MAPSIZE_BIG_Y)), nullptr));
			j++;
		}
	}

	return mobvec;
}

bool CMob::attacking() { return isAttacking; }

