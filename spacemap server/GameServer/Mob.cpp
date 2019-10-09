
#define _USE_MATH_DEFINES
#include <cmath>

#include "Mob.h"
#include "Session.h"
#include "DBUpdater.h"
#include "ResourceBox.h"


extern CDBGetter g_database_get;
extern CDBUpdater g_database_update;
extern std::mutex database_mutex;

extern std::map<id_t, std::mutex> g_emutex;

void CMob::handleWhoAmIGonnaShoot(damage_t dmg, id_t from)
{
	if (timeHasPassed(m_lastTimeShotByBelongedPlayer, 10000) // shooter-user has not been shooting since 10 seconds
		)
	{
		dcout << "Set focus to " << m_focusedPlayerId << cendl;
		m_isAttacking = true;
		m_focusedPlayerId = from;
	}
}

CMob::CMob(id_t mobid, const CMobWrapper& wrapper, Position_t pos, std::reference_wrapper<CSession> session,bool respawns)
	: CMob(mobid, wrapper.getShip(), wrapper.getName(), wrapper.getLoot(), wrapper.getReward(), wrapper.getAggressiveType(), pos, session,respawns)
{
}
CMob::CMob(id_t mobid, const Ship& shipinfo, username_t name, const Loot& loot, const Reward& reward, unsigned short aggrType, Position_t pos, std::reference_wrapper<CSession> session,bool respawns)
	: m_mm(pos, shipinfo.spd), m_session(session),m_respawns(respawns)
{
	this->m_id = mobid;
	this->m_ship = shipinfo;
	this->m_name = name;
	this->m_loot = loot;
	this->m_reward = reward;
	this->m_aggressiveType = aggrType;
	/*if (session != nullptr)
	{
	std::string init_str = m_pm.spawnEnemy(m_id, m_ship.id, 1, "", name, pos.first, pos.second, 0, 0, 0, false, 0, 0, false);
	session->sendEveryone(init_str);
	move(pos.first, pos.second); //fix client bug
	}*/
}

void CMob::move(pos_t x, pos_t y)
{
	//dcout << "Move id " << id << cendl;
	m_mm.get_current_position();
	m_mm.move(x, y);
	long long destinationTime = m_mm.get_time_for_destination();
	std::string move_str = m_pm.move(m_id, m_mm.get_current_position_x(), m_mm.get_current_position_y(), x, y, destinationTime);
	m_session.get().sendEveryone(move_str);
}

void CMob::attack(id_t uid)
{
	m_selectedId = uid;

	std::string attack_str = m_pm.laserAttack(m_id, m_selectedId, 0);
	m_session.get().sendEveryone(attack_str);
}

void CMob::abort()
{
	std::string abort = m_pm.laserAbort(m_id, m_selectedId);
	m_session.get().sendEveryone(abort);
}

void CMob::spawn(id_t user_id)
{
	handlePtr h = m_session.get().getHandler(user_id);
	if (h)
	{
		Position_t pos = m_mm.get_current_position();
		Position_t plan = m_mm.get_planned_position();
		std::string init_str = m_pm.spawnEnemy(m_id, m_ship.id, 1, "", m_name, pos.first, pos.second, 0, 0, 0, false, 0, 0, false);
		long long tfd = m_mm.recalc_time_for_destination();
		std::string move_str = m_pm.move(m_id, pos.first, pos.second, plan.first, plan.second, tfd);
		h->sendPacket(init_str);
		h->sendPacket(move_str);
	}
}
/* Initial spawn */
void CMob::spawn()
{
	Position_t pos = m_mm.get_current_position();
	Position_t plan = m_mm.get_planned_position();
	std::string init_str = m_pm.spawnEnemy(m_id, m_ship.id, 1, "", m_name, pos.first, pos.second, 0, 0, 0, false, 0, 0, false);
	long long tfd = m_mm.recalc_time_for_destination();
	std::string move_str = m_pm.move(m_id, pos.first, pos.second, plan.first, plan.second, tfd);
	m_session.get().sendEveryone(init_str);
	m_session.get().sendEveryone(move_str);

}

void CMob::generateRandomWaitingTime(long long timeNow, long long min, long long max)
{
	long long generated = random<long long>(min, max);
	m_waitForNextMoveTime = timeNow + generated + m_mm.get_time_for_destination();
}

void CMob::setRealWaitingTime(long long timeNow, long long val)
{
	m_waitForNextMoveTime = timeNow + val;
}

inline void CMob::generateRandomWaitingTime(long long timeNow,long long max)
{
	generateRandomWaitingTime(timeNow, 0, max);
}


id_t CMob::getId() { return m_id; }

Position_t CMob::getPosition() { return m_mm.get_current_position(); }

void CMob::setSession(std::reference_wrapper<CSession> session)
{
	this->m_session = session;
}

void CMob::setFocusToPlayer(id_t from)
{
	m_focusedPlayerId = from;
	if (m_aggressiveType == 1  /* aggressive enemy */
		&& from != 0/* when reset */
		)
	{
		m_isAttacking = true;
	}
	else
	{
		m_isAttacking = false; /* reset to true on switching when not aggresive*/
	}
}

id_t CMob::getFocusedPlayer()
{
	return m_focusedPlayerId;
}

id_t CMob::getBelongedPlayer()
{
	return m_belongToId;
}

void CMob::updateLootOwner(damage_t dmg, id_t shooterid)
{
	dcout << m_id << " UpdateLootOwner(" << dmg << "," << shooterid << ") - " << m_shareable << cendl;
	if (!m_shareable) // not "cubikon"
	{
		//flawless work on first try, something must be wrong
		if ((m_lastTimeShotByBelongedPlayer + 10000 < getTimeNow() || m_belongToId == 0) &&
			m_belongToId != shooterid)
		{
			/* Belong to new shooter */

			m_belongToId = shooterid;
			//m_lastTimeShotByBelongedPlayer = getTimeNow();
			dcout << "Greying Opponent to anyone but " << shooterid << cendl;
			m_greyed = true;
			m_session.get().sendEveryone(m_pm.greyOpponent(m_id, shooterid));
		}
		else if (m_belongToId == shooterid)
		{
			/* Update time for current shooter*/
			//m_lastTimeShotByBelongedPlayer = getTimeNow();
			dcout << "Greying Opponent from attacker " << shooterid << cendl;
			m_greyed = true;
			m_session.get().sendEveryone(m_pm.greyOpponent(m_id, shooterid));
		}
		else if (shooterid == 0)
		{
			/* Reset to "no-owner" */
			dcout << "shooterid is 0" << cendl;
			// id 0 = reset
			m_focusedPlayerId = 0;
			m_lastTimeShotByBelongedPlayer = 0LL;
		}
	}
	else
	{
		//TODO: make array and stuff
	}
}

damage_t CMob::receiveDamagePure(damage_t dmg, id_t from)
{
	updateLootOwner(dmg, from);
	handleWhoAmIGonnaShoot(dmg, from);
	if (from == m_belongToId && from != 0)
	{
		updateLastTimeShotByBelongedPlayer();
	}
	updateLastTimeShot();

	constexpr const double SHIELDPERCENTAGE = 0.5;
	/* Damage split between shield and HP */
	damage_t shield_damage = static_cast<damage_t>(dmg * SHIELDPERCENTAGE);
	damage_t health_damage = dmg - shield_damage; //fight me, if you want to have a (1 - SHIELDPERCENTAGE) 

												  /* HP / SHD after hit */
	health_t health_post_damage = m_ship.hp - health_damage;
	shield_t shield_post_damage = m_ship.shd - shield_damage;

	if (shield_post_damage < 0) {
		health_post_damage += shield_post_damage; //s_p_d is negative so we add that to our health 
		m_ship.shd = 0;
	}
	else {
		m_ship.shd = shield_post_damage;
	}

	if (health_post_damage <= 0) {
		m_ship.hp = 0;
		die();
		return -dmg; //ded
	}
	else {
		m_ship.hp = health_post_damage;
		return dmg; //no ded
	}
}

damage_t CMob::receiveDamageHP(damage_t dmg, id_t from)
{
	updateLootOwner(dmg, from);
	handleWhoAmIGonnaShoot(dmg, from);
	if (from == m_belongToId && from != 0)
	{
		setLastTimeShotByBelongedPlayer(getTimeNow());
	}
	damage_t realdmg = dmg;
	health_t hp = m_ship.hp - dmg;

	// to ensure no overflow happens
	if (hp <= 0) {
		realdmg += hp; // 5000 DMG 4000 HP -> -1000 thus 5000 + (-1000) = 4000
		m_ship.hp = 0;
		die();
	}
	else
	{
		m_ship.hp = hp;
	}

	return realdmg;
}
void CMob::rewardLoot()
{
	if (!m_shareable)
	{
		if (m_belongToId != 0)//&& m_lastTimeShotByFirstPlayer + 10000 < getTimeNow())
		{
			// CRED>URI>HON>XP
			handlePtr user = m_session.get().getHandler(m_belongToId);
			if (user != nullptr)
			{
				struct {
					long long credits_loot; //loot
					long long credits_total; //total
					long long uri_loot;
					long long uri_total;
					long long honor_loot;
					long long honor_total;
					long long experience_loot;
					long long experience_total;
				} w;

				w.credits_loot = m_reward.credits;
				w.credits_total = user->addCredits(m_reward.credits);
				w.uri_loot = m_reward.uri;
				w.uri_total = user->addUri(m_reward.uri);
				w.honor_loot = m_reward.honor;
				w.honor_total = user->addHonor(m_reward.honor);
				w.experience_loot = m_reward.exp;
				w.experience_total = user->addEP(m_reward.exp);

				try {
					std::string queryRaw =
						"UPDATE cuentas "
						"SET creditos = creditos + " + to_string(w.credits_loot) + ","
						"uridium = uridium + " + to_string(w.uri_loot) + ","
						"experiencia = experiencia + " + to_string(w.experience_loot) + ","
						"honor = honor + " + to_string(w.honor_loot) + " "
						"WHERE id = " + to_string(m_belongToId);
					g_database_update.queryUpdateRaw(queryRaw);
					user->sendPacket(m_pm.receiveLoot("CRE", { w.credits_loot,w.credits_total }));
					user->sendPacket(m_pm.receiveLoot("URI", { w.uri_loot,w.uri_total }));
					user->sendPacket(m_pm.receiveLoot("HON", { w.honor_loot,w.honor_total }));
					user->sendPacket(m_pm.receiveLoot("EP", { w.experience_loot,w.experience_total,(long long)user->getLevel() }));
				}
				catch (mysql_exception& me)
				{
					warncout << "User " << m_belongToId << " mysql_exception: " << me.what() << cendl;
					user->disconnectUser();
				}
			}
			else
			{
				dcout << "User on receiveLoot not found: " << m_belongToId << cendl;
			}
		}
	}
}

void CMob::dropResources()
{
	std::shared_ptr<CResourceBox> resource = std::make_shared<CResourceBox>(m_loot, m_session.get().generateNewCollectableId(), m_mm.get_current_position_x(), m_mm.get_current_position_y(), m_session, m_belongToId);
	m_session.get().addCollectable(resource);
}
CSession::NpcContainer_t::iterator CMob::die()
{
	std::lock_guard<std::mutex> l(m_mob_mutex);
	m_session.get().sendEveryone(m_pm.kill(m_id));
	m_session.get().sendEveryone(m_pm.removeOpponent(m_id));
	rewardLoot();
	dropResources();
	CSession::NpcContainer_t::iterator newit = m_session.get().removeMob(m_id);

	if (m_respawns)
	{
		if (m_ship.id != 80 && m_ship.id != 81) // TODO POLYMORPHISM
		{
			// respawn not cubikons / prot
			m_ship.hp = m_ship.hpmax;
			m_ship.shd = m_ship.shdmax;
			std::shared_ptr<CMob> newMobPostMortum = std::make_shared<CMob>(m_session.get().generateNewMobId(), CMobWrapper(*this), Position_t(random<pos_t>(m_session.get().getMap().getWidth()),
				random<pos_t>(m_session.get().getMap().getHeight())), std::ref(m_session),m_respawns);
			m_session.get().addMob(newMobPostMortum);
			newMobPostMortum->spawn();
		}
		else
		{

		}
	}
	return newit;
}

damage_t CMob::receiveDamageSHD(damage_t dmg, id_t from)
{
	updateLootOwner(dmg, from);
	handleWhoAmIGonnaShoot(dmg, from);
	if (from == m_belongToId && from != 0)
	{
		setLastTimeShotByBelongedPlayer(getTimeNow());
	}

	damage_t realdmg = dmg;
	shield_t shd = m_ship.shd - dmg;

	// to ensure no overflow happens
	if (shd <= 0) {
		realdmg += shd; // 5000 DMG 4000 HP -> -1000 thus 5000 + (-1000) = 4000
		m_ship.shd = 0;
	}
	else
	{
		m_ship.shd = shd;
	}

	return realdmg;
}

void CMob::increaseSHD(shield_t shd)
{
	m_ship.shd = std::min(m_ship.shd + shd, m_ship.shdmax);
}

void CMob::increaseHP(health_t hp)
{
	m_ship.hp = std::min(m_ship.hp + hp, m_ship.hpmax);
}

CMobWrapper CMob::getMobInfoFromDB(shipid_t alienship_id)
{
	CMobWrapper mob;
	CMob::Ship ship;
	CMob::Loot loot;
	CMob::Reward reward;
	std::string name;
	unsigned short aggressiveType;
	bool share;

	ship.id = alienship_id;
	std::lock_guard<std::mutex> l(database_mutex);
	{
		g_database_get.changeQuery("nombre", "npcs", "gfx", alienship_id);
		name = g_database_get.getString();

		g_database_get.changeQuery("pv", "npcs", "gfx", alienship_id);
		ship.hpmax = g_database_get.getInt();
		ship.hp = ship.hpmax;
		g_database_get.changeQuery("esc", "npcs", "gfx", alienship_id);
		ship.shdmax = g_database_get.getInt();
		ship.shd = ship.shdmax;

		//rewards
		g_database_get.changeQuery("exp", "npcs", "gfx", alienship_id);
		reward.exp = g_database_get.getUInt64();
		g_database_get.changeQuery("hon", "npcs", "gfx", alienship_id);
		reward.honor = g_database_get.getUInt64();
		g_database_get.changeQuery("cre", "npcs", "gfx", alienship_id);
		reward.credits = g_database_get.getUInt64();
		g_database_get.changeQuery("uri", "npcs", "gfx", alienship_id);
		reward.uri = g_database_get.getUInt64();


		// reward
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

		g_database_get.changeQuery("share", "npcs", "gfx", alienship_id);
		share = g_database_get.getBoolean();
	}
	mob.setAggressiveType(aggressiveType);
	mob.setLootShareable(share);
	mob.setLoot(loot);
	mob.setReward(reward);
	mob.setName(name);
	mob.setShip(ship);
	return mob;
}

std::vector<std::shared_ptr<CMob>> CMob::generateMobsForMap(map_t mapid, std::reference_wrapper<CSession> session)
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
			mobvec.push_back(
				std::make_shared<CMob>(
					BEGIN_MOB_IDS + j + MOB_IDS_ADD_PER_MAP * mapid, // mobid
					mobInformation[wr.shipid], // info
					Position_t(random<pos_t>(session.get().getMap().getWidth()),
						random<pos_t>(session.get().getMap().getHeight())), // position
					session, // session
					true) // respawns
			);
			j++;
		}
	}

	return mobvec;
}

bool CMob::attacking() { return m_isAttacking; }

void CMob::fastTick(long long timeNow, CSession& currentSession)
{
	//if alien wants to make another move (when the time has come)
	Position_t p = getPosition();
	//highest cpu take i guess
	id_t triggeredId = 0;
	if (getShip().id != 80)
	{
		triggeredId = getFocusedPlayer();
		if (!triggeredId) //dont handle cubikonikons
		{
			currentSession.foreachConnections([=,&triggeredId,&timeNow](id_t id, handlePtr& player){
					if (player != nullptr)
					{
						pos_t a = std::abs(player->getX() - p.first);
						pos_t b = std::abs(player->getY() - p.second);
						dist_t distance = static_cast<dist_t>(std::sqrt(a * a + b * b));
						if (player->m_pbIsInvisible == false && player->isInPeaceZone() == false && distance < Constants::Game::FIGHT_RANGE_NPC)
						{
							setFocusToPlayer(player->getID());
							setRealWaitingTime(timeNow, 0); // so it instantly flies towards enemy
							triggeredId = player->getID();
						}
					}
				}
			);

		}
	}
	if (timeHasPassed(timeNow, getLastTimeShot(), 60000))
	{
		//TODO SetHP SetSHD
		increaseHP(getShip().hpmax * 0.05);
	}
	if (getNextMovingTime() < timeNow)
	{

		handlePtr userIfTriggered;
		if (triggeredId)
			//Handle "players"
		{
			if (userIfTriggered = currentSession.getHandler(triggeredId))
			{
				pos_t a = std::abs(userIfTriggered->getX() - p.first);
				pos_t b = std::abs(userIfTriggered->getY() - p.second);
				dist_t distance = static_cast<dist_t>(std::sqrt(a * a + b * b));

				const dist_t DISTANCE_MAX_TILL_FOLLOW = 300;
				const dist_t DISTANCE_MAX_TILL_CIRCLE = 220;
				const dist_t DISTANCE_FOLLOW = 20;
				const dist_t DISTANCE_CLOSE = 200;
				if (distance > DISTANCE_MAX_TILL_FOLLOW)
				{
					//hey lets get a bit closer to our beloved friend enemy

					//for now its random, in the future make the degree with the shortest distance
					// (point enemy <-> point self    just make 2 points degree ez bye
					auto pos = getPosition();
					auto mypos = Position_t(userIfTriggered->getX(), userIfTriggered->getY());
					double degree = random<uint32_t>(360) * M_PI / 180.0; //atan2(mypos.second - pos.second, mypos.first - pos.first);
					decltype(pos) newmobpos = std::make_pair(mypos.first, mypos.second);
					//x
					newmobpos.first += DISTANCE_CLOSE * std::cos(degree);
					newmobpos.second += DISTANCE_CLOSE * std::sin(degree);
					move(newmobpos.first, newmobpos.second);
					setRealWaitingTime(timeNow, 100);// 0.1 sec w8
				}
				else if (distance > DISTANCE_MAX_TILL_CIRCLE)
				{
					double degree = random<uint32_t>(360) * M_PI / 180.0;
					auto pos = getPosition();
					auto mypos = Position_t(userIfTriggered->getX(), userIfTriggered->getY());
					decltype(pos) newmobpos = std::make_pair(mypos.first, mypos.second);
					//x
					newmobpos.first += DISTANCE_CLOSE * std::cos(degree);
					newmobpos.second += DISTANCE_CLOSE * std::sin(degree);
					move(newmobpos.first, newmobpos.second);
					setRealWaitingTime(timeNow, 1500);// 0.1 sec w8
				}
				else if (distance < DISTANCE_FOLLOW * 1.5)
				{
					//* TOO CLOSE * /
					auto pos = getPosition();
					auto mypos = Position_t(userIfTriggered->getX(), userIfTriggered->getY());
					unsigned int degree = random<uint32_t>(360) * M_PI / 180.0;
					decltype(pos) newmobpos = std::make_pair(mypos.first, mypos.second);
					//x
					newmobpos.first += DISTANCE_CLOSE * std::cos(degree);
					newmobpos.second += DISTANCE_CLOSE * std::sin(degree);
					move(newmobpos.first, newmobpos.second);
					setRealWaitingTime(timeNow, 1500);// 0.1 sec w8
				}
				else
				{
					//* MOVEMENT IN RANGE * /
					setRealWaitingTime(timeNow, 1500);
				}
			}
			else
			{
				//enemy "misteroiusly" disappeared im not gonna fix this typo its 5 am let me be
				triggeredId = 0;
				setFocusToPlayer(0);
			}
		}
		else {
			//handle "roaming"
			/* TODO: FIX ON RADIATION ZONE */
			int randx = 0;
			int randy = 0;
			const int RANGE = 10000;
			do {
				randx = random<pos_t>(-RANGE / 2, RANGE / 2) + p.first;
				randy = random<pos_t>(-RANGE / 2, RANGE / 2) + p.second;
			} while (randx > currentSession.getMap().getWidth() || randy > currentSession.getMap().getHeight() || randx < 0 || randy < 0);

			move(randx, randy);
			generateRandomWaitingTime(timeNow, 2500, 15000);

		}
	}
}

void CMob::secondTick(long long timeNow, CSession& currentSession)
{
	if (getFocusedPlayer() > 0 && getFocusedPlayer() < BEGIN_MOB_IDS &&
		attacking())
	{
		auto player = currentSession.getHandler(getFocusedPlayer());
		/* Abort if player on demilitarized zone */
		if (player != nullptr && player->isInPeaceZone()) 
		{
			abort();
			setLastTimeShotByBelongedPlayer(0LL);
			setFocusToPlayer(0);
		}
		
		if (player) {
			Position_t alienpos = getPosition();
			Position_t userpos =
				std::make_pair(player->getX(),
					player->getY());
			pos_t x = alienpos.first - userpos.first;
			pos_t y = alienpos.second - userpos.second;
			dist_t distance = static_cast<dist_t>(std::sqrt(x * x + y * y));
			if (distance < Constants::Game::FIGHT_RANGE_NPC)
			{
				/* IN RANGE */
				damage_t damage = m_dm.damageLaser(getId(), 1, getShip().dmg, false);

				//player->sendPacket(pm.damageBubbleSelf()
				if (player->receiveDamagePure(damage) >= 0)
				{
					/* Player is alive and healthy and is able to get attacked */
					attack(getFocusedPlayer());
				}
				else
				{
					/* Player is ded */
					player->die();
					abort();
					setLastTimeShotByBelongedPlayer(0LL);
					setFocusToPlayer(0);
				}

			}
			else
			{
				//no player founderino
				//dcout << "out of range but its notout of range but something i wrote too late in the night" << cendl;

				//stop the lasers. I have yet to find the packet that keeps "Locking" on a player but doesnt shoot lasers/whatever on him
				abort();
				//setLastTimeShotByFirstPlayer(0LL);
				//setTriggerPerson(0);
			}
		}
	}
	if (isGreyed() && getLastTimeShotByBelongedPlayer() + 10000 < timeNow) // iterated 4000 times gettimenow() in fast tick and wonder why this function took so much time 
	{

		dcout << "Ungrey opponent: " << cendl;
		setLastTimeShotByBelongedPlayer(0); //* Kinda unncessary but who cares * /
		ungrey();
		currentSession.sendEveryone(m_pm.ungreyOpponent(getId()));
	}
	if (timeHasPassed(timeNow, getLastTimeShot(), 10000))
	{
		//TODO SetHP SetSHD
		increaseSHD(getShip().shdmax * 0.05);
	}
}

