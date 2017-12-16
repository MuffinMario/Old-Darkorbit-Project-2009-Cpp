#include "PlayerHandler.h"

#define DBG_PACKETCLI
CDBGetter g_database_get("olddorps", "usuario", "cuentas", "id", 0); //default initalize
CDBUpdater g_database_update("olddorps", "usuario", "cuentas", "id",0); //default initalize
std::map<int, CSessionsManager> CPlayerHandler::m_sAllInSession;
std::map<map_t, CMap> g_allMaps;
CFileWriter g_filewrite("C:/xampp/htdocs/log.txt");
extern std::map<id_t, std::mutex> g_emutex;

///////////////////////
//////////////////////
////////////////////
/////////////////////
//////////////////////



CPlayerHandler::~CPlayerHandler()
{
	//TODO: REMEMBER NOT TO PUT LEAVESESSION HERE OMG HOW STUPID CAN I BE
	//flushThreads();
	cout << EColor::GREEN << "[SUCCESS] User " << m_id << " disconnected!" << cendl;
	if (m_id != 0)
	{
		DBUtil::funcs::setMap(m_id, getMapId());
		DBUtil::funcs::setPos(m_id, m_mm->get_current_position());
		g_filewrite.writemore("User " + to_string(m_id) + " logged off.", "Auth");
	}
	m_socket.close();
}

auto CPlayerHandler::createConnection(boost::asio::io_service & io_service, unsigned short port) -> CPlayerHandler::tcppointer
{
	
	CPlayerHandler *newhandle = new CPlayerHandler(io_service, port, &m_sAllInSession[0]);
	return tcppointer(newhandle);
}

tcp_t::socket & CPlayerHandler::getSocket()
{
	return m_socket;
}

void CPlayerHandler::start()
{
	readData();
}
void CPlayerHandler::login()
{
	
	m_bLoginSent = true;
	logInPackets();
	cout << EColor::GREEN << "[SUCCESS] User " << m_player.username << " (SessionId: " << m_sessionId << ") logged in." << cendl;
	g_filewrite.writemore("User " + to_string(m_id) + " logged in.", "Auth");

}
void CPlayerHandler::readData() {
	boost::asio::async_read_until(m_socket, m_buffer,"\n",BASIC_BIND_READER(CPlayerHandler));
	//move received stream from m_buffer into m_information string to handle packet
	//boost::asio::async_read_until(m_socket, m_buffer, "\n",
	//	boost::bind(&CPlayerHandler::handle_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}
bool CPlayerHandler::sendPacket(std::string str) {

#define USE_ASYNC 1
	std::string packet = str + (char)0;
	boost::asio::streambuf sendbuffer;
	std::ostream os(&sendbuffer);
	os << packet;
	try {
#if USE_ASYNC == 1
		boost::asio::async_write(m_socket,
			sendbuffer.data(),
			boost::bind(&CPlayerHandler::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)); // async because performance > sync operations
#else
		boost::asio::write(m_socket,
			sendbuffer.data()); // prevents packets to get sent in the wrong order (possibly)
#endif
	}
	catch (boost::exception& ex)
	{
#ifdef _DEBUG
		cout << EColor::RED << BOOST_CERR_OUTPUT(ex) << cendl;
#endif
		return false;
	}
	return true;
}
bool CPlayerHandler::sendPackets(std::vector<std::string> strs)
{
#undef USE_ASYNC
#define USE_ASYNC 1
	std::vector<boost::asio::streambuf*> streambuffers;

	for (std::string& str : strs)
	{
		std::string packet = str + (char)0;
		boost::asio::streambuf* sendbuffer = new boost::asio::streambuf();
		std::ostream os(sendbuffer);
		os << packet;
		streambuffers.push_back(sendbuffer);
	}

	long long before = getTimeNow();
	for (auto& sendbuffer : streambuffers) {
		long long before = getTimeNow();
		try {
#if USE_ASYNC == 1
			boost::asio::async_write(m_socket,
				sendbuffer->data(),
				boost::bind(&CPlayerHandler::handle_write, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)); // async because performance > sync operations
#else
			boost::asio::write(m_socket,
				sendbuffer->data()); // prevents packets to get sent in the wrong order (possibly)
#endif
		}
		catch (boost::exception& ex)
		{
#ifdef _DEBUG
			cout << EColor::RED << BOOST_CERR_OUTPUT(ex) << cendl;
#endif
			return false;
		}
		delete sendbuffer;
		long long delta = getTimeNow() - before;
		dcout << "Sent packet in " << delta << " ms." << cendl;
	}
	long long delta = getTimeNow() - before;
	dcout << "All in " << delta << " ms." << cendl;
	return true;
}
void CPlayerHandler::sendPacketAfter(int ms, std::string str) {
	m_asyncThreads.push_back(async_func(ms, &CPlayerHandler::sendPacket, str));
}

//packet sending handler, NOTE: UNUSED WHEN BOOST::ASIO::ASYNC_WRITE IS NOT USED
void CPlayerHandler::handle_write(const boost::system::error_code & ec, size_t bytes) { }

void CPlayerHandler::disconnectUser() {
	//m_socket.shutdown(boost::asio::m_socketbase::shutdown_type::shutdown_both);
	cout << EColor::LIGHT_BLUE << "[TEST] Closing socket" << cendl;
	cout << EColor::LIGHT_BLUE << "[TEST] Deleting Async Threads..." << cendl;
	m_socket.close();
	m_pbHasISH = false;
	m_pbIsAttacking = false;
	m_pbIsJumping = false;
	m_pbIsInSMBCooldown = false;
	m_bLoginSent = false;
	flushThreads();
	m_currentSession->leaveSession(m_sessionId);
	sendEveryone(m_sendpacket.removeOpponent(m_id), m_currentMap.getMapId());
}

bool CPlayerHandler::packetIs(std::string str) {
	return packetIsLevel(str, 0);
}
bool CPlayerHandler::packetIsLevel(std::string str, size_t level) {
	bool ret = m_splitpackets.at(level) == str; //EARLIER: boost::algorithm::starts_with(m_information, str); NOW TOTAL CHECK OF splitpackets[0]
	return ret;
}

//returns true of player is still alive
bool CPlayerHandler::makeDamage(damage_t dmg, handlePtr enemy) {
	return enemy->receiveDamagePure(dmg) >= 0;
}
bool CPlayerHandler::makeDamage(damage_t dmg, std::shared_ptr<CMob> mob) {
	return mob->receiveDamagePure(dmg) >= 0; //TODO IDK JUST CONTINUE
}

damage_t CPlayerHandler::makeSAB(damage_t dmg, handlePtr enemy) {
	damage_t realdmg = enemy->receiveDamageSHD(dmg);

	//add shd to current player
	shield_t p_shd = m_player.shd;
	if ((m_player.shd += realdmg) > m_player.maxshd)
	{
		m_player.shd = m_player.maxshd;
	}
	if (p_shd != m_player.shd)
	{
		updateShield();
	}

	return realdmg;
}
damage_t CPlayerHandler::makeSAB(damage_t dmg, std::shared_ptr<CMob> mob) {
	damage_t realdmg = mob->receiveDamageSHD(dmg);

	//add shd to current player
	shield_t p_shd = m_player.shd;
	if ((m_player.shd += realdmg) > m_player.maxshd)
	{
		m_player.shd = m_player.maxshd;
	}
	if (p_shd != m_player.shd)
	{
		updateShield();
	}

	return realdmg;
}


void CPlayerHandler::handleAttackRequest(id_t uid)
{
	if (!m_bIsAttackThreadRunning) {
		m_bIsAttackThreadRunning = true;
		m_asyncThreads.push_back(async_func(&CPlayerHandler::handleAttack,uid));
	}

}
void CPlayerHandler::handleAttack(id_t uid) /*noexcept*/ {
	//todo add jump handling
	m_pbIsAttacking = true;
	size_t previousDistance = 0;
	while (m_pbIsAttacking)
	{
		if (uid < BEGIN_MOB_IDS) //user is a player
		{
			try {
				handlePtrIt pair_handle = m_currentSession->getHandler(uid);
				handlePtr enemy = pair_handle->second;
				pos_t x = m_mm->get_current_position_x() - enemy->getX();
				pos_t y = m_mm->get_current_position_y() - enemy->getY();
				size_t distance = std::sqrt(x*x + y*y);
				if (distance <= Constants::Game::FIGHT_RANGE_PLAYER)
				{
					if(previousDistance > Constants::Game::FIGHT_RANGE_PLAYER)
						sendPacket("0|X");
					previousDistance = distance;
					damage_t damage = !enemy->m_pbHasISH ? m_damageManager.damageLaser(m_id, m_lasertype, 4000, false) : 0;
					//set health of enemy by damage taken
					if (m_lasertype != 5) // IF LASER IS NOT SAB
					{
						m_pbIsAttacking = makeDamage(damage, enemy);
					}
					else //if laser is sab
					{
						//damage now is according to shield amount of enemy (e.g 5000 SHD & 10000 SAB-DAMAGE  -> 5000 DAMAGE)
						damage = makeSAB(damage, enemy);
					}

					//bubbles
					health_t hp = enemy->getHP();
					shield_t shd = enemy->getSHD();
					sendPacket(m_sendpacket.damageBubbleSelected(hp, shd, damage, false));
					enemy->sendPacket(m_sendpacket.damageBubbleSelf(hp, shd, damage));

					enemy->updateRepairPrevent();
				}
				else
				{

					if (previousDistance <= Constants::Game::FIGHT_RANGE_PLAYER)
						sendPacket("0|O");
					previousDistance = distance;
				}
				//SO USER CANT REPAIR AFTER SHOOTING (for a specific time)
				updateRepairPrevent();
			}
			catch (std::out_of_range oor)
			{
				m_pbIsAttacking = false;
			}
		}
		else
		{
			dcout << "1" << cendl;
			std::shared_ptr<CMob> mob = m_currentSession->getMob(uid);
			if (mob != nullptr)
			{
				Position_t p = mob->getPosition();
				pos_t px = p.first, py = p.second;
				//Debugz
				dcout << px << "|" << py << cendl;
				static long long id = 2000000;
				sendPacket("0|r|" + to_string(id++) + "|2|" + to_string(px) + "|" + to_string(py));
				sendPacket("0|2|" + to_string(id - 10));

				pos_t x = m_mm->get_current_position_x() - px;
				pos_t y = m_mm->get_current_position_y() - py;
				size_t distance = std::sqrt(x*x + y*y);
				dcout << "2" << cendl;
				if (distance <= Constants::Game::FIGHT_RANGE_PLAYER)
				{
					if (previousDistance > Constants::Game::FIGHT_RANGE_PLAYER)
						sendPacket("0|X");
					previousDistance = distance;
					std::string laser_visuals = m_sendpacket.laserAttack(m_id, uid, m_damageManager.laserTypeToColor(m_lasertype, 3)); //TODO 3 -> oState dynamiczzz
					sendEveryone(laser_visuals, m_currentMap.getMapId());
					//TODO: if npc has ish
					
					damage_t damage = m_damageManager.damageLaser(m_id, m_lasertype, 4000, false);
					//set health of enemy by damage taken
					if (m_lasertype != 5) // IF LASER IS NOT SAB
					{
						m_pbIsAttacking = makeDamage(damage, mob);
					}
					else //if laser is sab
					{
						//damage now is according to shield amount of enemy (e.g 5000 SHD & 10000 SAB-DAMAGE  -> 5000 DAMAGE)
						damage = makeSAB(damage, mob);
					}

					//bubbles
					health_t hp = mob->getShip().hp;
					shield_t shd = mob->getShip().shd;
					sendPacket(m_sendpacket.damageBubbleSelected(hp, shd, damage, false));

					dcout << "3" << cendl;
					//SO USER CANT REPAIR AFTER SHOOTING (for a specific time)
				}
				else//outoffightingrange
				{

					if (previousDistance <= Constants::Game::FIGHT_RANGE_PLAYER)
						sendPacket("0|O");
					previousDistance = distance;
				}
				updateRepairPrevent();
			}
			else //mob is nptr
			{
				dcout << "4" << cendl;
				m_pbIsAttacking = false;
			}
		}
		
		if (!m_pbIsAttacking)
		{
			dcout << "6" << cendl;
			std::string abort = m_sendpacket.laserAbort(m_id, uid);
			sendEveryone(abort, m_currentMap.getMapId());
		}
		boost::this_thread::sleep_for(boost::chrono::seconds(1));
	}

	dcout << "7" << cendl;
	//it will indeed send twice if mob dies (not self aborted) but due to m_bisattackthreadrunning there is absolutely no reason this should do any problems
	std::string abort = m_sendpacket.laserAbort(m_id, uid);
	sendEveryone(abort, m_currentMap.getMapId());
	m_bIsAttackThreadRunning = false;
}

void CPlayerHandler::detonateSMB()
{
	m_pbIsInSMBCooldown = false; 
	sendPacket("0|A|CLR|SMB");
}

void CPlayerHandler::detonateISH()
{
	m_pbHasISH = false;
	// After 10 secs: ISH item available again
	std::this_thread::sleep_for(std::chrono::seconds(7));
	sendPacket("0|A|CLR|ISH");
}

map_t CPlayerHandler::getUserIDMapId(id_t id) {
	
	// returns the MAP ID of the user with the id "id"
	for (auto& it : m_sAllInSession) {
		for (auto& conns : it.second.getAllConnections()) {
			if (conns.second->getID() == id)
				return conns.second->getMapId();
		}
	}
	return 0;
}
bool CPlayerHandler::checkSelectedID(id_t id)
{
	
	//after a jump the id resets -> check if the selectedOpponent equals the attacker (f.e) and also the id must be NOT 0
	if (m_selectedOpponent == id && m_selectedOpponent != 0) {
		int currentMapID = m_currentMap.getMapId();
		int opponentMapID = getUserIDMapId(id);
		return opponentMapID == currentMapID && opponentMapID != 0; // 0 -> not found
	};
	return false;
}

void CPlayerHandler::logInPackets() {
	//preparing for jump
	m_player.shipid = DBUtil::funcs::getShip(m_id);
	m_player.isadmin = DBUtil::funcs::isAdmin(m_id);
	m_player.speed = DBUtil::funcs::getSpeed(m_id);
	m_player.shd = DBUtil::funcs::getSHD(m_id);
	m_player.maxshd = DBUtil::funcs::getmaxSHD(m_id);
	m_player.hp = DBUtil::funcs::getHP(m_id);
	m_player.maxhp = DBUtil::funcs::getmaxHP(m_id);
	m_player.cargospace = 100;
	m_player.cargospacemax = 500;
	m_player.mapid = DBUtil::funcs::getMap(m_id);
	m_player.fractionid = DBUtil::funcs::getCompany(m_id);
	m_player.clanid = DBUtil::funcs::getUserClanId(m_id);
	m_player.clantag = DBUtil::funcs::getClanTag(m_player.clanid);
	m_player.maxbatt = 20000;
	m_player.maxrock = 400;
	m_player.oState = 3;
	m_player.premium = true;
	m_player.exp = 5000120;
	m_player.hon = 400100;
	m_player.lvl = 42;
	m_player.cred = 1337420;
	m_player.uri = 69420;
	m_player.jackpot = 500.20f;
	if (m_player.isadmin == false)
	{
		m_player.rank = DBUtil::funcs::getRank(m_id);
	}
	else
	{
		m_player.rank = 21;
	}
	m_player.gatesAchieved = 2;
	m_player.useSysfont = true;
	m_player.username = DBUtil::funcs::getUsername(m_id);
	m_player.drones = DBUtil::funcs::getDrones(m_id);



	Position_t pos = DBUtil::funcs::getPos(m_id);
	/*
	IN REMINDER WHAT COST ME 4 DAYS TO FIND; A NEW REWRITTEN DBUTIL THAT RETURNS A PAIR INSTEAD OF A STRING
	AND I'VE LITTERALY GAVE UP MULTIPLE TIMES TO FIND THIS

	RIP: split(std::vector<std::string>,Position_t,...) 2016-2016 u'll not be missed:
											^
	boost::algorithm::split(pos, DBUtil::funcs::getPos(id), boost::is_any_of("|"));
	*/

	m_mm.reset(new CMovementManager(pos.first, pos.second, m_player.speed));
	pos_t x = m_mm->get_current_position_x();
	pos_t y = m_mm->get_current_position_y();

	std::string initalisepacket = m_sendpacket.init(m_id, m_player.username, m_player.shipid, m_player.speed, m_player.shd, m_player.maxshd, m_player.hp,
		m_player.maxhp, m_player.cargospace, m_player.cargospacemax, x, y, m_player.mapid, m_player.fractionid, m_player.clanid, m_player.maxbatt,
		m_player.maxrock, m_player.oState, m_player.premium, m_player.exp, m_player.hon, m_player.lvl, m_player.cred, m_player.uri, m_player.jackpot, m_player.rank,
		m_player.clantag, m_player.gatesAchieved, m_player.useSysfont);
	std::string map_i = m_sendpacket.loadMap(m_player.mapid);
	std::string map_m = m_sendpacket.loadMiniMap(m_player.mapid);
	dcout << "size: " << m_sAllInSession.size() << " - found sID in map " << m_player.mapid << ": " << m_sAllInSession[m_player.mapid].containsSessionId(m_sessionId) << cendl;
	try {
		m_sAllInSession[m_player.mapid].joinSession(shared_from_this(),m_sessionId);
		m_currentSession = &m_sAllInSession[m_player.mapid];
	}
	catch (...)
	{
		std::cerr << "how can this throw an error! " << ::GetLastError() << std::endl;
	}

	sendPacket("0|A|SET|1|1|1|1|1|1|1|1|1|1|1|1|0|1|1|1|1|1|1|1|1|0|0|0|0");
	sendPacket("0|u|1");
	sendPacket("0|d|1");
	sendPacket(initalisepacket);
	sendPacket(map_i);
	sendPacket(map_m);
	sendPacket("0|B|2|3|4|7|8");
	sendPacket("0|3|1|2|3");
	sendPacket("0|S|CFG|1");
	if(m_player.isadmin)
		sendPacket("0|A|ADM|CLI|1");
	sendPacket("0|8");

	m_lasertype = 1;
	m_rockettype = ERocketType::R310;

	generateObjects(m_player.mapid);
	generatePlayer();
	generateAliens();
	checkForObjectsToInteract();

	std::string spawnMe = m_sendpacket.spawnEnemy(m_id, m_player.shipid, 3, m_player.clantag, m_player.username, x + 410, y + 300, m_player.fractionid, 220,m_player.rank, false, 1, 4);
	std::string moveFix = m_sendpacket.move(m_id, x, y, x, y, 0);
	std::string drones = "0|n|d|" + boost::lexical_cast<std::string>(m_id) + "|" + m_player.drones;

	sendEveryone(spawnMe,m_currentMap.getMapId());
	sendEveryone(moveFix , m_currentMap.getMapId());
	sendEveryone(drones, m_currentMap.getMapId());
}
void CPlayerHandler::jump(map_t wantedMapID, pos_t dest_x, pos_t dest_y) {
	
	m_mm->reset(dest_x, dest_y);
	//in case of update these variables will be kept 
	m_player.shipid = DBUtil::funcs::getShip(m_id);
	m_player.speed = DBUtil::funcs::getSpeed(m_id);
	m_player.mapid = wantedMapID;
	m_player.fractionid = DBUtil::funcs::getCompany(m_id);
	m_player.clanid = DBUtil::funcs::getUserClanId(m_id);
	m_player.clantag = DBUtil::funcs::getClanTag(m_player.clanid);
	m_player.drones = DBUtil::funcs::getDrones(m_id);
	m_player.username = DBUtil::funcs::getUsername(m_id);
	if (m_player.isadmin == false)
	{
		m_player.rank = DBUtil::funcs::getRank(m_id);
	}
	else
	{
		m_player.rank = 21;
	}
	m_mm->set_speed(m_player.speed);

	std::string removeme = m_sendpacket.removeOpponent(m_id);
	std::string ss = m_sendpacket.displayStarSystem();
	std::string initalisepacket = m_sendpacket.init(m_id, m_player.username, m_player.shipid, m_player.speed, m_player.shd,
		m_player.maxshd, m_player.hp, m_player.maxhp, m_player.cargospace, m_player.cargospacemax, dest_x, dest_y, m_player.mapid,
		m_player.fractionid, m_player.clanid, m_player.maxbatt, m_player.maxrock, m_player.oState, m_player.premium, m_player.exp,
		m_player.hon, m_player.lvl, m_player.cred, m_player.uri, m_player.jackpot, m_player.rank, m_player.clantag, m_player.gatesAchieved, m_player.useSysfont);
	std::string map_i = m_sendpacket.loadMap(wantedMapID);
	std::string map_m = m_sendpacket.loadMiniMap(wantedMapID);

	std::string spawnMe = m_sendpacket.spawnEnemy(m_id, m_player.shipid, m_player.oState,m_player.clantag, m_player.username, dest_x + 410, dest_y + 300, m_player.fractionid, m_player.clanid, m_player.rank, false,1, 3);

		
	sendEveryone(removeme, m_currentMap.getMapId());
	//JUMP SESSION
	m_sAllInSession[m_currentMap.getMapId()].leaveSession(m_sessionId);
	m_sAllInSession[wantedMapID].joinSession(shared_from_this(),m_sessionId);
	m_currentSession = &m_sAllInSession[wantedMapID];
	//AFTER JUMP
	sendPacket(ss);
	sendPacket(initalisepacket);
	sendPacket(map_i);
	sendPacket(map_m);
	sendPacket("0|B|2|3|4|7|8");
	sendPacket("0|3|1|2|3");
	sendPacket("0|8");

	//also sets m_currentMap
	generateObjects(wantedMapID);
	generatePlayer();
	generateAliens();
	checkForObjectsToInteract();

	sendEveryone(spawnMe,m_currentMap.getMapId());
	sendEveryone("0|n|d|" + boost::lexical_cast<std::string>(m_id) + "|" + m_player.drones,m_currentMap.getMapId());

	m_pbIsJumping = false;

}

void CPlayerHandler::logoutHandle()
{
	uint64_t before_logout_sleep_time = m_last_logout_time;
	boost::this_thread::sleep_for(boost::chrono::seconds(m_player.premium ? Constants::Game::LOGOUT_TIME_PREMIUM : Constants::Game::LOGOUT_TIME));
	if (!m_blogout_cancel && m_last_logout_time == before_logout_sleep_time)
	{
		disconnectUser();
	}
}

void CPlayerHandler::updateHealth()
{
	static const auto fancyDBG = [&]() {
		double hpp = (double)m_player.hp/ (double)m_player.maxhp;
		double shdp = (double)m_player.shd / (double)m_player.maxshd;
		static const size_t BARS = 25;
		const size_t hpB = boost::math::round(BARS*hpp);
		const size_t shdB = boost::math::round(BARS*shdp);
		cout << "\tUser: " << m_player.username << "\n" << EColor::GREEN;
		for (size_t i = 0; i < hpB; i++)
		{
			cout << "|";
		}
		cout << EColor::GREY;
		for (size_t i = 0; i < BARS - hpB; i++)
		{
			cout << "|";
		}
		cout << "\n" << EColor::LIGHT_BLUE;
		for (size_t i = 0; i < shdB; i++)
		{
			cout << "|";
		}
		cout << EColor::GREY;
		for (size_t i = 0; i < BARS - shdB; i++)
		{
			cout << "|";
		}
		cout << cendl;
	};
	fancyDBG();
	sendPacket(m_sendpacket.updateHealth(m_player.hp, m_player.maxhp, m_player.shd, m_player.maxshd));
}
void CPlayerHandler::updateShield()
{
	sendPacket(m_sendpacket.updateSHD(m_player.shd, m_player.maxshd));
}
void CPlayerHandler::updateHitpoints()
{
	sendPacket(m_sendpacket.updateHP(m_player.hp, m_player.maxhp));
}

void CPlayerHandler::generateObjects(const map_t mapid) {
	
	CMap m = g_allMaps.find(mapid)->second;
	m_currentMap = m;
	generatePortals();
	generateStations();
}
void CPlayerHandler::generatePortals() {
	for (auto& it : m_currentMap.getPortals()) {
		std::string createdPortal = m_sendpacket.createPortal(it.portalIDonMap, 1, 1, it.x, it.y);
		sendPacket(createdPortal);
	}
}
void CPlayerHandler::generateStations() {
	for (auto& it : m_currentMap.getStations()) {
		std::string createdStation = m_sendpacket.createStation(it.stationIDonMap, 1, it.stationName, it.company, 1,
			it.x, it.y);
		sendPacket(createdStation);
	}
}
void CPlayerHandler::generatePlayer() {
	for (auto& it : m_currentSession->getAllConnections()) {
		if (it.first != m_sessionId)
		{
			id_t uid = it.second->getID();
			shipid_t shipID = boost::lexical_cast<shipid_t>(DBUtil::funcs::getShip(uid));
			factionid_t companyID = boost::lexical_cast<factionid_t>(DBUtil::funcs::getCompany(uid));
			Position_t pos = std::make_pair(it.second->getX() + 410, it.second->getY() + 300);
			rank_t rank = 0;
			if (DBUtil::funcs::isAdmin(uid))
			{
				rank = 21;
			}
			else
			{
				rank = boost::lexical_cast<rank_t>(DBUtil::funcs::getRank(uid));
			}
			std::string spawnstring = m_sendpacket.spawnEnemy(uid, shipID, 3, DBUtil::funcs::getUserClanTag(uid), DBUtil::funcs::getUsername(uid), pos.first, pos.second,
				companyID, DBUtil::funcs::getUserClanId(uid), rank, false, 1, 4);
			//std::string movestring = m_sendpacket.move(uid, pos.first-410, pos.second, pos.first, pos.second, 400, 0);
			sendPacket(spawnstring);
			//sendPacket(movestring);
			sendPacket("0|n|d|" + boost::lexical_cast<std::string>(uid) + "|" + DBUtil::funcs::getDrones(uid));
		}
	}
}
void CPlayerHandler::generateAliens() {
	int i = m_currentSession->getMobHighestId();
	while (i --> BEGIN_MOB_IDS)
	{
		g_emutex[i].lock();
		std::shared_ptr<CMob> mob = m_currentSession->getMob(i);
		if (mob != nullptr)
		{
			mob->spawn(m_id);
		}
		g_emutex[i].unlock();
	}
}
void CPlayerHandler::generatePlayer(id_t id) {
	try{
	handlePtrIt it = m_currentSession->getHandler(id);
		auto& opponentHandle = it->second;
		id_t uid = opponentHandle->getID();
		shipid_t shipID = DBUtil::funcs::getShip(uid);
		factionid_t companyID = DBUtil::funcs::getCompany(uid);
		rank_t rank = 0;
		if (DBUtil::funcs::isAdmin(uid))
		{
			rank = 21;
		}
		else 
		{
			rank = DBUtil::funcs::getRank(uid);
		}
			clanid_t clanID = DBUtil::funcs::getUserClanId(uid);
		std::string spawnstring = m_sendpacket.spawnEnemy(uid, shipID, 3, "PO", DBUtil::funcs::getUsername(opponentHandle->getID()), opponentHandle->getX() + 410, opponentHandle->getY() + 300,
			companyID, clanID, rank, false, 1, 4);

		sendPacket(spawnstring);
		sendPacket("0|n|d|" + boost::lexical_cast<std::string>(it->second->getID()) + "|" + DBUtil::funcs::getDrones(it->second->getID()));
	}
	catch (std::out_of_range oor)
	{
		std::cout << oor.what() << std::endl;
	}
}
void CPlayerHandler::checkForObjectsToInteract() {
	
	int portalID = 0;
	bool closeToGate = false;
	bool closeToStation = false;

	//for the sake of multiple read/writes these two for each loops have to create temporary new "Portals/Stations" instead of iterating through them.
	for (auto& it : m_currentMap.getPortals()) {
		if (isInRangeCircle(CPortal::RANGE, it.x, it.y)) {
			portalID = it.portalIDonMap;
			closeToGate = true;
			break;
		}
	}
	for (auto& it : m_currentMap.getStations()) {
		if (isInRangeSquare(CStation::RANGE, it.x - 510, it.y - 300)) {
			closeToStation = true;
			break;
		}
	}
	//TO-DO add other check, etc.

	std::string packet = m_sendpacket.events(closeToGate ? 1 : closeToStation ? 1 : 0, 0, closeToStation, 0, closeToGate, 0);
	sendPacket(packet);
}
bool CPlayerHandler::isInRangeCircle(int radius, pos_t objx, pos_t objy) {
	
	//must admit i couldnt really come up with this myself, guess this might be because its in the middle of the night right now, feel so ashamed :D
	Position_t position(m_mm->get_current_position());
	pos_t dx = std::abs(position.first - objx);
	if (dx > radius) return false;
	pos_t dy = std::abs(position.second - objy);
	if (dy > radius) return false;
	if (dx + dy <= radius) return true;
	//slower check if nothing of that other stuff happens (compiler safety)
	return (position.first - objx)*(position.first - objx) + (position.second - objy)*(position.second - objy) < radius*radius;
}
bool CPlayerHandler::isInRangeSquare(int radius, pos_t objx, pos_t objy)
{
	
	Position_t position(m_mm->get_current_position());
	if (position.first < objx - radius) {
		return false;
	}
	if (position.first > objx + radius) {
		return false;
	}
	if (position.second < objy - radius) {
		return false;
	}
	if (position.second > objy + radius) {
		return false;
	}
	return true;
}
void CPlayerHandler::handle_read(const boost::system::error_code & ec, size_t bytes)
{
	switch (ec.value()) {
	case 0:
		break;
	case boost::asio::error::eof:
	case boost::asio::error::connection_reset:
		cout << EColor::YELLOW << "[Warning] User connection lost: " << m_id << cendl;
		disconnectUser();
		return;
	case 1236:
		//disconnected by e.g. attack handler from other player
		return;
	default:
		//dcout << "[Warning] Unknown error \"" << ec.value() << "\" at " << __FUNCTION__ << "(" << __LINE__ << "). Closing connection." << cendl;
		dcout << "[Warning] Unknown exception message: " << ec.message() << cendl;
		//disconnectUser();
		return;
	}
	std::istream is(&m_buffer);
	std::getline(is, m_information);

	handle_read_b(bytes);
}

//due to boost::bind not allowing overloading of bound functions, we must rename this
void CPlayerHandler::handle_read_b(size_t bytes)
{
	m_last_ping_time = getTimeNow();
	if (bytes > 0) {
		using namespace Constants::Received; // incoming packet headers
		using namespace Constants; // for send, as it is not often used
		using boost::lexical_cast;
		//cout << EColor::YELLOW << "[INFO] Packet -> " << m_information << cendl;
		fixPacket();

		//for debug packets
		static int dbgvar = 13000;

		boost::algorithm::split(m_splitpackets, m_information, boost::is_any_of("|"));
		try { //try for bad_lexical_cast globally stopping packet check
			if (m_bLoginSent == false)
			{
				if (packetIs(W_POLICYFILEREQUEST)) {
					sendPacket(Send::W_CROSSDOMAINPOLICY);
				}
				else if (packetIs(LOGIN_REQUEST))
				{
					m_id = lexical_cast<id_t>(m_splitpackets.at(1));
					m_sessionId = m_splitpackets.at(2);
					login();
				}
			}
			else {
				if (packetIs(LOGIN_REQUEST)) {
					dcout << "But papa login is already sent" << cendl;
				}
				else if (packetIs("0")) {
					if (m_player.rank == 21)
					{
						if (packetIsLevel("T", 1))
						{
							pos_t gx = lexical_cast<pos_t>(m_splitpackets.at(2));
							pos_t gy = lexical_cast<pos_t>(m_splitpackets.at(3));
							m_mm->move(gx, gy);
						}
						sendPacket(m_information);
					}
				}
				else if (packetIs("A")) {
					//received: S|1|1|0|...
					//send: 0|S|1|1|0|..., we do not do anything with these informations yet
					if (packetIsLevel(A_SETTINGS, 1))
					{
						sendPacket("0|" + m_information);
					}
				}
				else if (packetIs(MOVE)) {
					//repair stop on move for small time
					updateRepairPrevent();
					//determining the local and planned position

					/* LOGOUT BUTTON - SECURITY*/
					if (m_splitpackets.at(1) != "NaN" && m_splitpackets.at(2) != "NaN") {
						pos_t px = lexical_cast<pos_t>(m_splitpackets.at(1));
						pos_t py = lexical_cast<pos_t>(m_splitpackets.at(2));

						m_mm->move(px, py);
						Position_t position(m_mm->get_current_position());
						std::string move = m_sendpacket.move(m_id, position.first, position.second, px, py, m_mm->get_time_for_destination());
						sendEveryone(move, m_currentMap.getMapId());
						checkForObjectsToInteract();
					}
					else
					{
						auto npos = std::string::npos;
						// NaN | NaN | NaN | NaN -> error in system, currently happening when you move to a position on the minimap where youre already heading (rarely, idk why exactly)
						// Nan | Nan | 123 | 123 -> user logout, last position on 123|123
						if (m_splitpackets.at(3).find("Infinity") != npos
							|| m_splitpackets.at(3).find("NaN") != npos 
							|| m_splitpackets.at(4).find("Infinity") != npos
							|| m_splitpackets.at(4).find("NaN") != npos){
							g_filewrite.writemore("User " + to_string(m_id) + " MOVE REQUEST BUG \"" + m_information + "\"", "Client"); //client error
							disconnectUser();
						}
						else
						{
							//TODO: Save pos to DB
							savePosToDB();
						}
						
					}
				}
				else if (packetIs(LOGOUT))
				{
					toggleLogoutCancel(false);
					updateLogoutTime();
					m_asyncThreads.push_back(async_func(&CPlayerHandler::logoutHandle));
				}
				else if (packetIs(LOGOUT_CANCEL))
				{
					toggleLogoutCancel(true);
				}
				else if (packetIs(JUMP)) {
					if (!m_pbIsJumping) {
						Position_t position = m_mm->get_current_position();
						/*static int oreval = 205000;
						std::string ore_id = lexical_cast<std::string>(oreval++);
						std::string id = lexical_cast<std::string>(3);
						std::string pos_x = lexical_cast<std::string>(position.first);
						std::string pos_y = lexical_cast<std::string>(position.second);
						sendPacket("0|r|" + ore_id + "|" + id + "|" + pos_x + "|" + pos_y);*/
						// make this a function u lazy ass
						std::shared_ptr<CPortal> myPortal{ nullptr };
						for (auto& it : m_currentMap.getPortals()) {
							if (isInRangeCircle(500, it.x, it.y)) {
								m_pbIsJumping = true;
								myPortal.reset(new CPortal(it.portalIDonMap, it.x, it.y, it.destination_x, it.destination_y, it.destination_mapID));
								break;
							}
						}
						if (m_pbIsJumping) {
							sendPacket("0|U|" + lexical_cast<std::string>(myPortal->destination_mapID) + "|" + lexical_cast<std::string>(myPortal->portalIDonMap));
							m_asyncThreads.push_back(async_func(2000, &CPlayerHandler::jump, myPortal->destination_mapID, myPortal->destination_x, myPortal->destination_y));
						}
					}
				}
				else if (packetIs(LASERSHOOT_START)) {
					id_t uid = 0;
					uid = lexical_cast<id_t>(m_splitpackets.at(1));
					if (!m_pbIsAttacking) {
						if (uid >= BEGIN_MOB_IDS || checkSelectedID(uid)) { // >= 120.000 = NPC
							//m_pbIsAttacking = true;
							handleAttackRequest(uid);
						}
					}
				}
				else if (packetIs(ROCKETSHOOT)) {
					//KEEP IN MIND THIS IS NOT CHEAT-PROOF AND I AM TOO LAZY TO CHANGE IT WITH TIMESTAMPS
					id_t uid = 0;

					sendPacketAfter(2000, "0|A|RCD");
					uid = lexical_cast<id_t>(m_splitpackets.at(1));
					if (uid < BEGIN_MOB_IDS) // PLAYER
					{
						//PLAYER
						if (checkSelectedID(uid)) {
							damage_t damage = 0;
							std::string shoot;
							std::string bubbleMe;
							std::string bubbleEnemy;
							try {
								const handlePtrIt opponentHandle = m_currentSession->getHandler(uid);
								handlePtr enemy = opponentHandle->second;
								if (!enemy->m_pbHasISH)
									damage = m_damageManager.damageRocket(uid, m_rockettype);
								shoot = m_sendpacket.rocketAttack(m_id, uid, m_rockettype, damage); //if damage == 0 hit is false
								if (makeDamage(damage, enemy)) //enemy didnt die
								{
									bubbleMe = m_sendpacket.damageBubbleSelected(enemy->getHP(), enemy->getSHD(), damage, false);
									bubbleEnemy = m_sendpacket.damageBubbleSelf(enemy->getHP(), enemy->getSHD(), damage);
									sendEveryone(shoot, m_currentMap.getMapId());
									sendPacket(bubbleMe);
									enemy->sendPacket(bubbleEnemy);
								}
								else //enemy died
								{

								}

								updateRepairPrevent();
								enemy->updateRepairPrevent();
							}
							catch (std::out_of_range oor) {}

						}
					}
					else // NPC
					{
						//NPC
						std::shared_ptr<CMob> mob = m_currentSession->getMob(uid);
						if (mob != nullptr)
						{
							damage_t damage = 0;
							std::string shoot;
							std::string bubbleMe;
							//if (!mob->m_pbHasISH)
							damage = m_damageManager.damageRocket(uid, m_rockettype);
							shoot = m_sendpacket.rocketAttack(m_id, uid, m_rockettype, damage); //if damage == 0 hit is false
							if (makeDamage(damage, mob)) //enemy didnt die
							{
								bubbleMe = m_sendpacket.damageBubbleSelected(mob->getShip().hp, mob->getShip().shd, damage, false);
								sendEveryone(shoot, m_currentMap.getMapId());
								sendPacket(bubbleMe);
							}

							updateRepairPrevent();

						}

					}
				}
				else if (packetIs(LASERAMMO_CHANGE)) {
					lasertype_t laserid = 0;
					laserid = lexical_cast<lasertype_t>(m_splitpackets.at(1));
					//ELaserColor = color			lasertype_t = ammo
					if (laserid > 5 || laserid < 1)
					{
						g_filewrite.writemore("USERID: " + lexical_cast<std::string>(m_id) + " ILLEGAL ACTVITY! USER CHOSE AMMO TYPE WHICH DOES NOT EXIST!","Illegal");
						cout << EColor::RED << "WARNING: " << m_id << " used illegal m_lasertype " << laserid << cendl;
						laserid = boost::algorithm::clamp(laserid, 1, 5);
					}
					m_lasertype = laserid;
					if (m_pbIsAttacking) {
						//ELaserColor laserColor = CDamageManager::laserTypeToColor(m_lasertype, m_player.oState);
						//std::string attackLaser = m_sendpacket.laserAttack(m_id, m_selectedOpponent, laserColor);
						//sendEveryone(attackLaser, m_currentMap.getMapId());
					}
				}
				else if (packetIs(ROCKETAMMO_CHANGE)) {
					rockettype_t rocketid = 0;

					rocketid = lexical_cast<rockettype_t>(m_splitpackets.at(1));
					if (rocketid > 3 || rocketid < 1)
					{
						g_filewrite.writemore("USERID: " + lexical_cast<std::string>(m_id) + " ILLEGAL ACTVITY! USER CHOSE ROCKET TYPE WHICH DOES NOT EXIST!","Illegal");
						cout << EColor::RED << "WARNING: " << m_id << " used illegal m_rockettype " << rocketid << cendl;
						rocketid = boost::algorithm::clamp(rocketid, 1, 3);
					}
					m_rockettype = static_cast<ERocketType>(rocketid);
				}
				else if (packetIs(LASERSHOOT_ABORT)) {
					m_pbIsAttacking = false;
				}
				else if (packetIs(SPECIAL))
				{

					if (packetIsLevel(SPECIAL_SMARTBOMB, 1)) {
						//high priority
						m_pbIsInSMBCooldown = true;
						sendPacket("0|A|CLD|SMB|10");

						std::string userID_string = lexical_cast<std::string>(m_id);
						sendEveryone("0|n|SMB|" + userID_string, m_currentMap.getMapId());

						m_asyncThreads.push_back(async_func(10000, &CPlayerHandler::detonateSMB));


						//race condition prevention
						if (m_pbIsInSMBCooldown)
						{
							for (auto& it : m_currentSession->getAllConnections()) {
								auto& opponentHandle = it.second;
								if (isInRangeCircle(500, opponentHandle->getX(), opponentHandle->getY()) &&
									!(opponentHandle->m_pbHasISH) && opponentHandle->getID() != m_id &&
									opponentHandle->getID() < BEGIN_MOB_IDS) {
									opponentHandle->receiveDamageHP(opponentHandle->getHP() * 0.2);
								}
							}
						}
					}
					else if (packetIsLevel(SPECIAL_INSTASHIELD, 1)) {
						//high priority
						m_pbHasISH = true;
						sendPacket("0|A|CLD|ISH|10");

						std::string userID_string = lexical_cast<std::string>(m_id);
						sendEveryone("0|n|ISH|" + userID_string, m_currentMap.getMapId());

						m_asyncThreads.push_back(async_func(3000, &CPlayerHandler::detonateISH));

					}

					else if (packetIsLevel(SPECIAL_REPBOT, 1))
					{
						auto now = getTimeNow();
						if (getTimeNow() - 3000 > m_last_repair_prevention) // 5 seconds cooldown
						{
							m_player.hp = m_player.maxhp;
							sendPacket(m_sendpacket.updateHP(m_player.hp, m_player.maxhp));
						}
						else
						{
							//idk some predetermined message maybe?
						}
					}
				}
				else if (packetIs(SELECT)) {
					m_selectedOpponent = lexical_cast<id_t>(m_splitpackets.at(1));
					id_t uid = m_selectedOpponent;
					if (uid < BEGIN_MOB_IDS)
					{
						if (checkSelectedID(uid)) {
							handlePtrIt opponentHandle;
							std::string selectShowBars;
							health_t normal_hp_opp = 100;
							health_t max_hp_opp = 100;
							shield_t normal_shd_opp = 100;
							shield_t max_shd_opp = 100;

							try {
								opponentHandle = m_currentSession->getHandler(uid);

								normal_hp_opp = opponentHandle->second->getHP();
								max_hp_opp = opponentHandle->second->getMaxHP();
								normal_shd_opp = opponentHandle->second->getSHD();
								max_shd_opp = opponentHandle->second->getMaxSHD();

								selectShowBars = m_sendpacket.showBars(normal_hp_opp, max_hp_opp, normal_shd_opp, max_shd_opp);
								sendPacket(selectShowBars);
							}
							catch (std::out_of_range oor) {}

						}
					}
					else //alienios
					{
						std::shared_ptr<CMob> mob = m_currentSession->getMob(uid);
						if (mob != nullptr)
						{
							CMob::Ship mobship = mob->getShip();
							std::string selectedShowBars = m_sendpacket.showBars(mobship.hp, mobship.hpmax, mobship.shd, mobship.shdmax);
							sendPacket(selectedShowBars);
						}
					}
				}
				else if (packetIs(FORCEINIT)) {
					if (!m_pbIsJumping)
					{
						id_t init_id = lexical_cast<id_t>(m_splitpackets.at(1));
						if (init_id >= BEGIN_MOB_IDS)
						{
							//later check if mob is CLOSE to char instead of spawning all mobs global on map
							std::shared_ptr<CMob> m = m_currentSession->getMob(init_id);
							m->spawn(m_id); //send spawn packets
						}
						else
						{
							generatePlayer(init_id);
						}
					}
				}
#ifdef DBG_PACKETCLI
				else if (m_player.isadmin)
				{
					if (packetIs("DBG_PACKETTEST")) {
						sendMessagePacket("Currently no packet test ongoing. :(");
					}
					else if (packetIs("DBG_PACKETTEST2")) {
						sendMessagePacket("Currently no 2nd packet test ongoing. :(");
					}
					else if (packetIs("DBG_KICK"))
					{
						sendEveryone(m_sendpacket.kill(m_id), m_currentMap.getMapId());
						disconnectUser();
					}
					else if (packetIs("DBG_MOVE_X"))
					{
						pos_t right = lexical_cast<pos_t>(m_splitpackets.at(1));
						if (m_selectedOpponent >= BEGIN_MOB_IDS)
						{
							std::shared_ptr<CMob> mob = m_currentSession->getMob(m_selectedOpponent);
							if (mob != nullptr)
							{
								Position_t pos = mob->getPosition();
								mob->move(pos.first + right, pos.second);
								mob->generateRandomWaitingTime(10000, 10000);
							}
						}
					}
					else if (packetIs("DBG_MOVE_Y"))
					{
						pos_t down = lexical_cast<pos_t>(m_splitpackets.at(1));
						if (m_selectedOpponent >= BEGIN_MOB_IDS)
						{
							std::shared_ptr<CMob> mob = m_currentSession->getMob(m_selectedOpponent);
							if (mob != nullptr)
							{
								Position_t pos = mob->getPosition();
								mob->move(pos.first, pos.second + down);
								mob->generateRandomWaitingTime(10000, 10000);
							}
						}
					}
					else if (packetIs("DBG_MOVE"))
					{
						pos_t right = lexical_cast<pos_t>(m_splitpackets.at(1));
						pos_t down = lexical_cast<pos_t>(m_splitpackets.at(2));
						if (m_selectedOpponent >= BEGIN_MOB_IDS)
						{
							std::shared_ptr<CMob> mob = m_currentSession->getMob(m_selectedOpponent);
							if (mob != nullptr)
							{
								Position_t pos = mob->getPosition();
								mob->move(pos.first + right, pos.second + down);
								mob->generateRandomWaitingTime(10000, 10000);
								std::thread([&](std::shared_ptr<CMob> mob) { 
									int rng = random<int>();
									Position_t pos = mob->getPosition();
									while (sendPacket(m_sendpacket.createOre(rng++,3,pos.first,pos.second)) && sendPacket(m_sendpacket.removeOre(rng - 2)))
									{
										std::this_thread::sleep_for(std::chrono::milliseconds(33));
										Position_t newpos = mob->getPosition();
										if (pos.first == newpos.first && pos.second == newpos.second)
										{
											break;
										}
										pos = newpos;
									}
						
								},mob).detach();
							}
						}
					}
					else if (packetIs("MAP")) {
						//provide a comfortable jump command for administreturs
						map_t wantedMapID = 1;
						pos_t dest_x = 0;
						pos_t dest_y = 0;

						try {
							wantedMapID = lexical_cast<map_t>(m_splitpackets.at(1));
							dest_x = lexical_cast<pos_t>(m_splitpackets.at(2));
							dest_y = lexical_cast<pos_t>(m_splitpackets.at(3));
						}
						catch (const std::out_of_range oor) {
							dest_x = m_mm->get_current_position_x();
							dest_y = m_mm->get_current_position_y();
						}
						catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
						catch (const boost::exception& e) { std::cerr << BOOST_CERR_OUTPUT(e) << std::endl; }
						jump(wantedMapID, dest_x, dest_y);
					}
					else if (packetIs("SPAWNIT")) { //usage SPAWNIT|<shipid>|<name>|<pos/H>|<pos/H>
						static id_t mobID = BEGIN_MOB_IDS + 1337420;
						shipid_t mob_shipID = 71;
						std::string mob_name = "<undefined>";
						pos_t mob_x = 410; //g_OffsetX = 410 -> for m_information see /frame 10/DoAction_30 in spacemap.swf
						pos_t mob_y = 300; //g_OffsetY = 300 -> for m_information see /frame 10/DoAction_30 in spacemap.swf

						try {
							/* Mob ShipType*/
							mob_shipID = lexical_cast<shipid_t>(m_splitpackets.at(1));
							if ((mob_shipID < 71 || mob_shipID > 90) && mob_shipID != 2 || mob_shipID != 3) {
								mob_shipID = 71;
							}

							/* Mob Name*/
							mob_name = "-=[ " + m_splitpackets.at(2) + " ]=-";


							/* Mob Pos */
							/*x*/
							if (m_splitpackets.at(3) == "H" || m_splitpackets.at(3) == "h") {
								mob_x += getX() + 1;
							}
							else {
								mob_x += lexical_cast<pos_t>(m_splitpackets.at(3));
							}
							/*y*/
							if (m_splitpackets.at(4) == "H" || m_splitpackets.at(4) == "h") {
								mob_y += getY() + 1;
							}
							else {
								mob_y += lexical_cast<pos_t>(m_splitpackets.at(4));
							}
						}
						catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
						catch (const boost::exception& e) { std::cerr << BOOST_CERR_OUTPUT(e) << std::endl; }


						std::string spawnmobstring = m_sendpacket.spawnEnemy(mobID++, mob_shipID,0, "", mob_name, mob_x, mob_y, 0, 0, 0, false, 0, 0, false);
						dcout << spawnmobstring << cendl;
						sendPacket(spawnmobstring);
					}
#ifdef A
					else if (packetIs("DBG_AI_BOOM"))
					{
						size_t i = m_currentSession->getMobHighestId();
						while (i-- > BEGIN_MOB_IDS)
						{
							std::shared_ptr<CMob> mob = m_currentSession->getMob(i);
							if (mob)
							{
								mob->die();
							}
						}
					}
					else if (packetIs("DBG_AI_M2"))
					{
						DBG_SIMULATE_AI_MOVE();
					}
					else if (packetIs("DBG_AI_MAGNET"))
					{
						const int distance = 250;

						size_t i = m_currentSession->getMobHighestId();
						while (i-- > BEGIN_MOB_IDS)
						{
							std::shared_ptr<CMob> mob = m_currentSession->getMob(i);
							if (mob)
							{
								unsigned int degree = random<uint32_t>(360);
								auto pos = mob->getPosition();
								auto mypos = m_mm->get_current_position();
								decltype(pos) newmobpos = std::make_pair(mypos.first,mypos.second);
								//x
								newmobpos.first += distance * std::cos(degree);
								newmobpos.second += distance * std::sin(degree);
								mob->move(newmobpos.first,newmobpos.second);
								mob->generateRandomWaitingTime(10000, 50000);
							}
						}

					}
					else if (packetIs("DBG_AI"))
					{
						DBG_SIMULATE_AI();
					}
#endif
					else if (packetIs("FLUSH_THREADS"))
					{
						flushThreads();
					}
				} //elseif-admin
#endif
			} //end packet startswith...
		}
		catch (std::out_of_range& oor)
		{
#ifdef _DEBUG
			dcout << "[EXCEPTION] Out of Range in vector in packet \"" << m_information << "\"" << cendl;
#endif
		}
		catch (boost::bad_lexical_cast blc)
		{
#ifdef _DEBUG
			dcout << "[EXCEPTION] Bad lexical cast in packet \"" << m_information << "\"" << cendl;
#endif
		}
	}//END OF IF BYTES > 0
	readData();
}

void CPlayerHandler::savePosToDB()
{

}

void CPlayerHandler::sendMessagePacket(std::string text)
{
	sendPacket(m_sendpacket.sendMessage(text));
}

void CPlayerHandler::sendEveryone(std::string packet,map_t map_id) {
	m_sAllInSession[map_id].sendEveryone(packet);
}

damage_t CPlayerHandler::receiveDamagePure(damage_t dmg)
{	
	constexpr const double SHIELDPERCENTAGE = 0.8;
	/* Damage split between shield and HP */
	damage_t shield_damage = dmg * SHIELDPERCENTAGE;
	damage_t health_damage = dmg - shield_damage; //fight me, if you want to have a (1 - SHIELDPERCENTAGE) 

	/* HP / SHD after hit */
	health_t health_post_damage = m_player.hp - health_damage;
	shield_t shield_post_damage = m_player.shd - shield_damage;

	if (shield_post_damage < 0) {
		health_post_damage += shield_post_damage; //s_p_d is negative so we add that to our health 
		m_player.shd = 0;
	}
	else {
		m_player.shd = shield_post_damage;
	}

	if (health_post_damage <= 0) {
		m_player.hp = 0;
		die();
		return -dmg; //ded
	}
	else {
		m_player.hp = health_post_damage;
		updateHealth();
		return dmg; //no ded
	}

}

damage_t CPlayerHandler::receiveDamageHP(damage_t dmg) {
	damage_t realdmg = dmg;
	health_t hp = m_player.hp - dmg;

	// to ensure no overflow happens
	if (hp <= 0) {
		realdmg += hp; // 5000 DMG 4000 HP -> -1000 thus 5000 + (-1000) = 4000
		m_player.hp = 0;
		die();
	}
	else
	{
		m_player.hp = hp;
	}

	updateHitpoints();
	return realdmg;
}

damage_t CPlayerHandler::receiveDamageSHD(damage_t dmg) {
	damage_t realdmg = dmg;
	shield_t shd = m_player.shd - dmg;

	// to ensure no overflow happens
	if (shd <= 0) {
		realdmg += shd; // 5000 DMG 4000 HP -> -1000 thus 5000 + (-1000) = 4000
		m_player.shd = 0;
	}
	else
	{
		m_player.shd = shd;
	}

	updateShield();
	return realdmg;
}

void CPlayerHandler::die()
{
	
	try {
		sendEveryone(m_sendpacket.kill(m_id), m_currentMap.getMapId());
		sendEveryone(m_sendpacket.removeOpponent(m_id),m_currentMap.getMapId());
		disconnectUser();
	}
	catch (boost::exception_ptr e) {
		std::cerr << BOOST_CERR_OUTPUT(e) << std::endl;
	}
	catch (std::exception e) {
		//std::cerr << STD_CERR_OUTPUT(e) << std::endl;
	}
}
//should happen at least at the end of a users time u know this joins and stuff
void CPlayerHandler::flushThreads()
{
	for (auto& thread : m_asyncThreads)
	{
#ifdef EXTREME_BRUTAL_THREAD_KILLER
		//doesnt delete shared_ptrs in threads so njiet spasibo
		TerminateThread(thread->native_handle(), 0);
#else
		if (thread->joinable())
		{
			thread->join();
		}
		else
		{
			thread->interrupt();
		}
#endif
		delete thread;
		thread = nullptr;
	}
	m_asyncThreads.clear();
}
void CPlayerHandler::fixPacket() {
	
	try {
		boost::trim_if(m_information, [](char c) { return c == 0x0; });
		boost::trim(m_information); // better, lol
	}
	catch (std::exception& ex) {
		std::cerr << STD_CERR_OUTPUT(ex) << m_information << std::endl;
	}
	catch (boost::exception& ex) {
		std::cerr << BOOST_CERR_OUTPUT(ex) << m_information << std::endl;
	}
}

#ifdef A
void CPlayerHandler::DBG_SIMULATE_AI()
{
	static id_t mobid_counter = 120000;
	for (size_t i = 0; i < 1500; i++)
	{
		std::shared_ptr<CMob> succ(new CMob(mobid_counter, CMob::Ship{ 2,4000,4000,4000,4000,20,280 }, "-=[ Streuner ]=-", CMob::Loot{ 1,2,3,4,10,11,12,20,21,30,40 }, false, Position_t(random<pos_t>(CMap::MAPSIZE_NORMAL_X), random<pos_t>(CMap::MAPSIZE_NORMAL_X)), m_currentSession));
		mobid_counter++;

		m_currentSession->addMob(succ);
	}
	DBG_SIMULATE_AI_MOVE();
}
void CPlayerHandler::DBG_SIMULATE_AI_MOVE()
{
	std::thread* notmemoryleakthread = new std::thread([](CSessionsManager* session) {
		int countr = 0;
		for (map_t map = 1; map < 30; map++)
		{
			auto session = m_sAllInSession[map];
			countr += session.getMobCount();
		}
		dcout << countr << " ALIENS FLYING AROUND FREELY WOO!" << cendl;
		const pos_t RANGE = 10000;
		for (;;)
		{
			for (map_t map = 1; map < 30; map++)
			{
				auto session = m_sAllInSession[map];
				id_t i = session.getMobHighestId();

				while (i --> BEGIN_MOB_IDS)
				{
					std::shared_ptr<CMob> mp = session.getMob(i);
					if (mp != nullptr)
					{
						if (mp->getNextMovingTime() < getTimeNow())
						{
							
							Position_t p = mp->getPosition();
							int randx = 0;
							int randy = 0;
							if (map != 16 && map != 29)
							{
								do {
									randx = random<pos_t>(-RANGE/2,RANGE/2) + p.first;
									randy = random<pos_t>(-RANGE / 2, RANGE / 2) + p.second;
								} while (randx > CMap::MAPSIZE_NORMAL_X || randy > CMap::MAPSIZE_NORMAL_Y || randx < 0 || randy < 0);
							}
							else
							{
								do {
									randx = random<pos_t>(-RANGE / 2, RANGE / 2) + p.first;
									randy = random<pos_t>(-RANGE / 2, RANGE / 2) + p.second;
								} while (randx > CMap::MAPSIZE_BIG_X || randy > CMap::MAPSIZE_BIG_Y || randx < 0 || randy < 0);
							}
							mp->move(randx, randy);
							mp->generateRandomWaitingTime(150, 15000);
						}
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(33)); // 30 Hz tickrate
			/* Profiling
				CPU Specs:
				i5-4460 @ 3.20 GHz (quad-Core)
									Debug-Mode

				10 ms: 19%~21% CPU on n = 500
				50 ms: 13%~15% CPU on n = 500

									Release-Mode
				0 ms: constant 25% on n = 1500 (3x 500)
				10 ms: ~0%-4% CPU on n = 500 (4% on rare occasions though [spikes only])
				33 ms: 0%-4% CPU on n = 1500 (3x 500) while youtube on 1080p was running it quite increased for some reason to almost always spiking up to 6% but i think 5% in total for only npcs is fairly OK
				50 ms: ~0% CPU on n = 500

				result: 30 Hz refresh rate nice and not shit cpu

				On average there should be about 30*50 NPCs (3x the amount of the test) on the server actively flying (30 maps ,ignoring personal GGs yet, inhabited by aliens)

			*/
		}
	}, m_currentSession);
}
#endif
template<class callable, class ...arguments>
boost::thread* CPlayerHandler::async_func(callable && func, arguments&&... args)
{
	boost::thread* thread_handle = new boost::thread(func, shared_from_this(), args...);
	//thread_handle->detach();
	return thread_handle;
}
template<class callable, class ...arguments>
boost::thread* CPlayerHandler::async_func(int ms,callable && func, arguments&&... args)
{
	auto f = boost::bind(func, shared_from_this(), args...);
	boost::thread* thread_handle = new boost::thread([f,ms]() {
		boost::this_thread::sleep_for(boost::chrono::milliseconds(ms));
		f();
	});
	//thread_handle->detach();
	return thread_handle;
}
