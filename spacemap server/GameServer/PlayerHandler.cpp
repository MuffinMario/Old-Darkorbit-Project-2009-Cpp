#include "PlayerHandler.h"
#include "ResourceBox.h"
#include "BonusBox.h"
#include <optional>

#define _USE_MATH_DEFINES
#include <math.h>

#define DBG_PACKETCLI
#define PRINT_PACKETS 0 //another flash related packet issue? set to 1 and you can start your horrible experience


CDBGetter g_database_get("olddorps", "usuario", "cuentas", "id", 0); //default initalize
CDBUpdater g_database_update("olddorps", "usuario", "cuentas", "id", 0); //default initalize

/* Contains all sessions. Meh... prefer it on global than static global */
CSessionsManager g_sessionsManager;

std::map<map_t, CMap> g_allMaps;
std::map<level_t, exp_t> g_levelTable;
std::map<level_t, exp_t> g_droneLevelTable;
std::map<shipid_t, CShipInfo> g_shipinfo;
std::map<rockettype_t, CRocketInfo> g_rocketinfo;
std::map<ore_t, COreInfo> g_oreinfo;

CFileWriter g_filewrite("log.txt");

extern std::map<id_t, std::mutex> g_emutex;

///////////////////////
//////////////////////
////////////////////
/////////////////////
//////////////////////



CPlayerHandler::~CPlayerHandler()
{
	//Don't put leaveSession here. this gets called if there are no references.

	kickcout << fancyPrintName() << " disconnected!" << cendl;
	if (m_id != 0)
	{
		g_filewrite.writemore("User " + to_string(m_id) + " logged off.", "Auth");
	}
	m_socket.close();
}

auto CPlayerHandler::createConnection(boost::asio::io_service& io_service, unsigned short port) -> CPlayerHandler::tcppointer
{

	CPlayerHandler* newhandle = new CPlayerHandler(io_service, port, nullptr);
	return tcppointer(newhandle);
}

tcp_t::socket& CPlayerHandler::getSocket()
{
	return m_socket;
}

bool CPlayerHandler::attackID(id_t uid)
{
	auto enemyHandle = m_currentSession->getHandler(uid);
	auto npcHandle = m_currentSession->getMob(uid);
	auto mapid = getMapId();
	if (uid < 12 && m_id >= 12)
	{
		sendMessagePacket("You can't attack a testing Unit!");
		return false;
	}
	else if (enemyHandle == nullptr && npcHandle == nullptr)
	{
		return false;
	}
	else if(enemyHandle != nullptr && enemyHandle->isInPeaceZone())
	{
		// TODO: temporary and company only
		auto peacezone = m_sendpacket.peaceArea();
		sendPacket(peacezone);
		enemyHandle->sendPacket(peacezone);
		return false;
	}
	else
	{
		if (!m_pbIsAttacking) {
			if (uid >= BEGIN_MOB_IDS || checkSelectedID(uid)) { // >= 120.000 = NPC
																//m_pbIsAttacking = true;

				handleAttackRequest(uid);
			}
		}
	}
	return true;
}

void CPlayerHandler::handleConfigChange(unsigned int toConfig)
{
}

void CPlayerHandler::onEPChange()
{
	constexpr const level_t MAX_LEVEL = 32;
	level_t currentLevel = m_player.lvl;
	// what if user gets from level 1 to 4 see it has to be repetitive i mean there are other ways but this is the one i went with
	while (currentLevel < MAX_LEVEL &&
		g_levelTable[++currentLevel] <= m_player.exp)
	{
		//hooray level up
		m_player.lvl = currentLevel;
		// has internal mutex
		try {
			std::string rawQuery =
				"UPDATE cuentas "
				"SET nivel = " + to_string(m_player.lvl) + " "
				"WHERE id = " + to_string(m_id);
			g_database_update.queryUpdateRaw(rawQuery);
			long long expForNext = 0LL;
			if (currentLevel < MAX_LEVEL) // g_levelTable[33] = not existent
			{
				expForNext = g_levelTable[currentLevel + 1] - m_player.exp;
			}
			if ((currentLevel != MAX_LEVEL && expForNext > 0LL) ||
				(currentLevel == MAX_LEVEL && expForNext >= 0LL))
			{
				sendPacket(m_sendpacket.levelUp(currentLevel, expForNext));
			}
		}
		catch (mysql_exception& me)
		{
			warncout << "User " << m_id << " mysql_exception in onEPChange(): " << me.what() << ". Disconnecting User..." << cendl;
			disconnectUser();
			return;
		}
	}
}
void CPlayerHandler::selectOpponent(id_t id)
{
	m_selectedTimeStamp = getTimeNow();
	m_previousSelectedOpponent = m_selectedOpponent;
	m_selectedOpponent = id;
}

void CPlayerHandler::start()
{
	readData();
}
bool CPlayerHandler::login()
{

	m_bLoginSent = true;
	health_t hp = DBUtil::funcs::getHP(m_id);
	map_t map = DBUtil::funcs::getMap(m_id);
	if (hp <= 0) // double check on logInPackets.
	{
		std::string nohperr = m_sendpacket.error_noHP();
		sendPacketSynchronous(nohperr);
		kickcout << "User " << m_id << " logged in with " << hp << " HP" << cendl;
		return false;
	}
	else if (map == 0 || g_sessionsManager.getAnySession(map).getHandler(m_id) != nullptr)
	{
		/* Message the newly logged in, or the one already logged in? This kicks the new logged in */
		std::string alreadyloggedin = m_sendpacket.error_alreadyLoggedIn();
		sendPacketSynchronous(alreadyloggedin);
		kickcout << "User " << m_id << " logged in already." << cendl;
		return false;
	}
	else
	{
		try {
			loginPlayer(); // also initiates m_mm, on kick saves position!
		}
		catch (const mysql_exception& me) {
			errorcout << "UserID " << m_id << " could not log in due to mysql_exception: " << me.what() << cendl;
			return false;
		}
		successcout << "User " << m_player.username << " (SessionId: " << m_sessionId << ") logged in." << cendl;
		g_filewrite.writemore("User " + to_string(m_id) + " logged in.", "Auth");
		return true;
	}
}
void CPlayerHandler::readData() {
	boost::asio::async_read_until(m_socket, m_buffer, "\n", BASIC_BIND_READER(CPlayerHandler)); // because we call this function only inside the call, this is basically a "synchronous" function.
	//move received stream from m_buffer into m_information string to handle packet
	//boost::asio::async_read_until(m_socket, m_buffer, "\n",
	//	boost::bind(&CPlayerHandler::handle_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}
#define USE_ASYNC 1
bool CPlayerHandler::sendPacket(const char* str)
{
#if PRINT_PACKETS == 1
	dcout << "sendPacket: " << str << cendl;
#endif
	boost::asio::streambuf sendbuffer;
	std::ostream os(&sendbuffer);
	os << str << (char)0;

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
		errorcout << BOOST_CERR_OUTPUT(ex) << cendl;
#endif
		return false;
	}
	return true;

}
bool CPlayerHandler::sendPacketSynchronous(std::string& str)
{
#if PRINT_PACKETS == 1
	dcout << "sendPacket: " << str << cendl;
#endif
	boost::asio::streambuf sendbuffer;
	std::ostream os(&sendbuffer);
	os << str << (char)0;
	try {
		boost::asio::write(m_socket,
			sendbuffer.data()); // prevents packets to get sent in the wrong order (possibly)
	}
	catch (boost::exception& ex)
	{
#ifdef _DEBUG
		errorcout << BOOST_CERR_OUTPUT(ex) << cendl;
#endif
		return false;
	}
	return true;
}
bool CPlayerHandler::sendPacketSynchronous(const char* str)
{
#if PRINT_PACKETS == 1
	dcout << "sendPacket: " << str << cendl;
#endif
	boost::asio::streambuf sendbuffer;
	std::ostream os(&sendbuffer);
	os << str << (char)0;
	try {
		boost::asio::write(m_socket,
			sendbuffer.data()); // prevents packets to get sent in the wrong order (possibly)
	}
	catch (boost::exception& ex)
	{
#ifdef _DEBUG
		errorcout << BOOST_CERR_OUTPUT(ex) << cendl;
#endif
		return false;
	}
	return true;
}
bool CPlayerHandler::sendPacket(const std::string& str) {
#if PRINT_PACKETS == 1
	dcout << "sendPacket: " << str << cendl;
#endif
	boost::asio::streambuf sendbuffer;
	std::ostream os(&sendbuffer);
	os << str << (char)0;
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
		errorcout << BOOST_CERR_OUTPUT(ex) << cendl;
#endif
		return false;
	}
	return true;
}
bool CPlayerHandler::sendPackets(std::initializer_list<std::string> strs)
{
#if PRINT_PACKETS == 1
	for(auto& str : strs)
		dcout << "sendPacket: " << str << cendl;
#endif

#undef USE_ASYNC
#define USE_ASYNC 1
	std::vector<boost::asio::streambuf*> streambuffers;

	for (const std::string& str : strs)
	{
		std::string packet = str + (char)0;
		boost::asio::streambuf* sendbuffer = new boost::asio::streambuf();
		std::ostream os(sendbuffer);
		os << packet;
		streambuffers.push_back(sendbuffer);
	}

	for (auto& sendbuffer : streambuffers) {
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
			errorcout << BOOST_CERR_OUTPUT(ex) << cendl;
#endif
			return false;
		}
		delete sendbuffer;
	}
	return true;
}
void CPlayerHandler::sendPacketAfter(int ms, std::string& str) {
	async_func(ms, static_cast<bool (CPlayerHandler::*)(const std::string&)>(&CPlayerHandler::sendPacket), std::ref(str));
}
void CPlayerHandler::sendPacketAfter(int ms, const char* str) {
	async_func(ms, static_cast<bool (CPlayerHandler::*)(const char*)>(&CPlayerHandler::sendPacket), str);
}

//packet sending handler, NOTE: UNUSED WHEN BOOST::ASIO::ASYNC_WRITE IS NOT USED
void CPlayerHandler::handle_write(const boost::system::error_code& ec, size_t bytes) { 
}

void CPlayerHandler::disconnectUser(bool updateDB) {
	//m_socket.shutdown(boost::asio::m_socketbase::shutdown_type::shutdown_both);
	/*
	m_pbHasISH = false;
	m_pbIsAttacking = false;
	m_pbIsJumping = false;
	m_pbIsInSMBCooldown = false;
	m_bLoginSent = false;
	*/
	dcout << __FUNCTION__ <<"("<<updateDB <<")"<< cendl;
	suspendThreads();
	if (m_currentSession)
		sendEveryone(m_sendpacket.removeOpponent(m_id));
	if (m_currentSession)
		m_currentSession->leaveSession(m_id);
	if (updateDB)
		db_updateOnLeave();
	m_socket.close();
}

bool CPlayerHandler::packetIs(const std::string& str) {
	return packetIsLevel(str, 0);
}
bool CPlayerHandler::packetIsLevel(const std::string& str, size_t level) {
	bool ret = m_splitpackets.at(level) == str; //EARLIER: boost::algorithm::starts_with(m_information, str); NOW TOTAL CHECK OF splitpackets[0]
	return ret;
}

//returns true of player is still alive
bool CPlayerHandler::makeDamage(damage_t dmg, handlePtr enemy) {
	return enemy->receiveDamagePure(dmg) >= 0;
}
bool CPlayerHandler::makeDamage(damage_t dmg, std::shared_ptr<CMob> mob) {
	return mob->receiveDamagePure(dmg, m_id) >= 0; //TODO IDK JUST CONTINUE
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
		updateShield(realdmg);
	}

	return realdmg;
}
damage_t CPlayerHandler::makeSAB(damage_t dmg, std::shared_ptr<CMob> mob) {
	damage_t realdmg = mob->receiveDamageSHD(dmg, m_id);

	//add shd to current player
	shield_t p_shd = m_player.shd;
	if ((m_player.shd += realdmg) > m_player.maxshd)
	{
		m_player.shd = m_player.maxshd;
	}
	if (p_shd != m_player.shd)
	{
		updateShield(realdmg);
	}

	return realdmg;
}


void CPlayerHandler::abortAttack()
{
	std::string abort = m_sendpacket.laserAbort(m_id, m_selectedOpponent);
	sendEveryone(abort);
}
void CPlayerHandler::handleAttackRequest(id_t uid)
{
	if (!m_bIsAttackThreadRunning) {
		m_bIsAttackThreadRunning = true;
		async_func(&CPlayerHandler::handleAttack, uid);
	}

}
void CPlayerHandler::cloak()
{
	m_pbIsInvisible = true;
	try {
		DBUtil::funcs::setInvisible(m_id, true);
		sendEveryone(m_sendpacket.cloak(m_id, 1));
	}
	catch (mysql_exception& me)
	{

		warncout << "MySQL Exception thrown @CPlayerHandler::cloak(). Disconnecting User " << m_id << cendl;
		disconnectUser();
		return;
	}
}
void CPlayerHandler::uncloak()
{
	m_pbIsInvisible = false;
	try {
		DBUtil::funcs::setInvisible(m_id, false);
		sendEveryone(m_sendpacket.cloak(m_id, 0));
	}
	catch (mysql_exception& me)
	{

		warncout << "MySQL Exception thrown @CPlayerHandler::cloak(). Disconnecting User " << m_id << cendl;
		disconnectUser();
		return;
	}
}
void CPlayerHandler::handleAttack(id_t uid) /*noexcept*/ {
	//todo add jump handling
	m_pbIsAttacking = true;
	CPacketComposer sendpacket;
	try {
		dist_t previousDistance = 0;
		while (m_pbIsAttacking)
		{
			boost::this_thread::interruption_point();
			if (uid < BEGIN_MOB_IDS) //user is a player
			{
				handlePtr enemy = m_currentSession->getHandler(uid);
				if (enemy)
				{
					pos_t x = m_mm->get_current_position_x() - enemy->getX();
					pos_t y = m_mm->get_current_position_y() - enemy->getY();
					dist_t distance = static_cast<dist_t>(std::sqrt(x * x + y * y));
					if (distance <= Constants::Game::FIGHT_RANGE_PLAYER)
					{
						/* IN RANGE */

						if (m_pbIsInvisible)
							uncloak();

						bool dead = false;
						if (previousDistance > Constants::Game::FIGHT_RANGE_PLAYER)
						{
							/* WAS NOT IN RANGE BEFORE */
							sendPacket("0|X");

						}
						previousDistance = distance;
						damage_t damage = !enemy->m_pbHasISH ? m_damageManager.damageLaser(m_id, m_lasertype, m_player.basedamage, false) : 0;
						//set health of enemy by damage taken
						if (m_lasertype != 5) // IF LASER IS NOT SAB
						{
							dead = !(m_pbIsAttacking = makeDamage(damage, enemy));
						}
						else //if laser is sab
						{
							//damage now is according to shield amount of enemy (e.g 5000 SHD & 10000 SAB-DAMAGE  -> 5000 DAMAGE)
							damage = makeSAB(damage, enemy);
						}
						std::string laser_visuals = sendpacket.laserAttack(m_id, uid, m_damageManager.laserTypeToColor(m_lasertype, 3)); //TODO 3 -> oState dynamiczzz
						sendEveryone(laser_visuals);

						//bubbles
						health_t hp = enemy->getHP();
						shield_t shd = enemy->getSHD();
						sendPacket(sendpacket.damageBubbleSelected(hp, shd, damage, false));

						if (dead) enemy->die();
					}
					else
					{
						/* NOT IN RANGE */
						if (previousDistance <= Constants::Game::FIGHT_RANGE_PLAYER)
						{
							/* WAS IN RANGE PREVIOUSLY*/
							sendPacket("0|O");
							enemy->sendPacket(CPacketer(Constants::Send::MSG_ESCAPEDTHEATTACK));
							sendEveryoneButMe(sendpacket.laserAbort(m_id, uid));
						}
						previousDistance = distance;
					}
					//SO USER CANT REPAIR AFTER SHOOTING (for a specific time)
					updateRepairPrevent();
					updateNAZPrevent();
					updateLogoutTime();
				}
				else
				{
					m_pbIsAttacking = false;
				}
			}
			else
			{
				std::shared_ptr<CMob> mob = m_currentSession->getMob(uid);
				if (mob != nullptr)
				{
					Position_t p = mob->getPosition();
					pos_t px = p.first, py = p.second;

					pos_t x = m_mm->get_current_position_x() - px;
					pos_t y = m_mm->get_current_position_y() - py;
					dist_t distance = static_cast<dist_t>(std::sqrt(x * x + y * y));
					if (distance <= Constants::Game::FIGHT_RANGE_PLAYER)
					{
						/* IN RANGE */

						if (m_pbIsInvisible)
							uncloak();

						bool dead = false;
						if (previousDistance > Constants::Game::FIGHT_RANGE_PLAYER)
							sendPacket("0|X");
						previousDistance = distance;
						std::string laser_visuals = sendpacket.laserAttack(m_id, uid, m_damageManager.laserTypeToColor(m_lasertype, 3)); //TODO 3 -> oState dynamiczzz
						sendEveryone(laser_visuals);
						//TODO: if npc has ish

						damage_t damage = m_damageManager.damageLaser(m_id, m_lasertype, m_player.basedamage, false);
						//set health of enemy by damage taken
						if (m_lasertype != 5) // IF LASER IS NOT SAB
						{
							dead = !(m_pbIsAttacking = makeDamage(damage, mob));
						}
						else //if laser is sab
						{
							//damage now is according to shield amount of enemy (e.g 5000 SHD & 10000 SAB-DAMAGE  -> 5000 DAMAGE)
							damage = makeSAB(damage, mob);
						}

						//bubbles
						health_t hp = mob->getShip().hp;
						shield_t shd = mob->getShip().shd;
						sendPacket(sendpacket.damageBubbleSelected(hp, shd, damage, false));

						if (dead)
						{
							//mob->die();
							updateRepairPrevent();
							updateLogoutTime();
							m_pbIsAttacking = false;
							break;
						}
						//SO USER CANT REPAIR AFTER SHOOTING (for a specific time)
					}
					else//outoffightingrange
					{

						if (previousDistance <= Constants::Game::FIGHT_RANGE_PLAYER)
							sendPacket("0|O");
						previousDistance = distance;
					}
					updateRepairPrevent();
					updateLogoutTime();
				}
				else //mob is nptr
				{
					m_pbIsAttacking = false;
				}
			}

			if (!m_pbIsAttacking)
			{
				std::string abort = m_sendpacket.laserAbort(m_id, uid);
				sendEveryone(abort);
			}
			boost::this_thread::sleep_for(boost::chrono::seconds(1));
		}
	}
	catch (boost::thread_interrupted&)
	{
		m_pbIsAttacking = false;
	}
	//it will indeed send twice if mob dies (not self aborted) but due to m_bisattackthreadrunning there is absolutely no reason this should do any problems (soon to be a scene of the past popping up of me saying this exact thing while hell is let loose due to this issue)
	std::string abort = m_sendpacket.laserAbort(m_id, uid);
	sendEveryone(abort);
	m_bIsAttackThreadRunning = false;
}

void CPlayerHandler::detonateSMB()
{
	// After 10 secs: SMB available again
	m_pbIsInSMBCooldown = false;
	sendPacket("0|A|CLR|SMB");
}

void CPlayerHandler::detonateISH(long long delta)
{
	// After 3 secs
	m_pbHasISH = false;
	// After 10 secs: ISH item available again
	std::this_thread::sleep_for(std::chrono::milliseconds(delta));
	sendPacket("0|A|CLR|ISH");
}

map_t CPlayerHandler::getUserIDisOnMap(id_t id) {

	// returns the MAP ID of the user with the id "id"
	m_currentSession->lockConnectionsRead();
	for (auto& conns : m_currentSession->getAllConnections()) {
		if (conns.second->getID() == id)
		{ /// well well well look who forgot the brackets
			m_currentSession->unlockConnectionsRead();
			return conns.second->getMapId();
		}
	}
	m_currentSession->unlockConnectionsRead();
	return 0;
}
bool CPlayerHandler::checkSelectedID(id_t id)
{

	//after a jump the id resets -> check if the selectedOpponent equals the attacker (f.e) and also the id must be NOT 0
	if (m_selectedOpponent == id && m_selectedOpponent != 0) {
		int currentMapID = m_currentMap.getMapId();
		int opponentMapID = getUserIDisOnMap(id);
		return opponentMapID == currentMapID && opponentMapID != 0; // 0 -> not found
	};
	return false;
}

void CPlayerHandler::loginPlayer() {
	try {
		m_mm.reset(new CMovementManager());
		
		initPlayerOnLogin();

		/* Build strings to be sent as packets */
		pos_t x = m_mm->get_current_position_x();
		pos_t y = m_mm->get_current_position_y();

		std::string initalisepacket = m_sendpacket.init(m_id, m_player.username, m_player.shipid, m_player.speed, m_player.shd, m_player.maxshd, m_player.hp,
			m_player.maxhp, m_player.cargospace, m_player.cargospacemax, x, y, m_player.mapid, m_player.factionid, m_player.clanid, m_player.maxbatt,
			m_player.maxrock, m_player.oState, m_player.premium, m_player.exp, m_player.hon, m_player.lvl, m_player.cred, m_player.uri, m_player.jackpot, m_player.rank,
			m_player.clantag, m_player.gatesAchieved, m_player.useSysfont);
		std::string map_i = m_sendpacket.loadMap(m_player.mapid);
		std::string map_m = m_sendpacket.loadMiniMap(m_player.mapid);

		std::string spawnMe = m_sendpacket.spawnEnemy(m_id, m_player.shipid, 3, m_player.clantag, m_player.username, x + 410, y + 300, m_player.factionid, 220, m_player.rank, false, 1, 4);
		std::string moveFix = m_sendpacket.move(m_id, x, y, x, y, 0);
		//TODO: settings.displaydrones 0|n|d true ; 0|n|e false
		std::string drones = m_sendpacket.initDronesInformational(m_id, 0, 8);
		std::string invis = m_sendpacket.cloak(m_id, m_pbIsInvisible);

		/* join (and leave) session */
		joinSession(m_player.mapid);
		
		/* Update player properties on map */
		checkForObjectsToInteract();

		/* Send build strings packets*/
		sendPacketSynchronous("0|A|SET|1|1|1|1|1|1|1|1|1|1|1|1|0|1|1|1|1|1|1|1|1|0|0|0|1");
		sendPacketSynchronous("0|u|1"); // TODO: dbutils chosen laser
		sendPacketSynchronous("0|d|1"); // TODO: dbutils chosen rocket
		sendPacketSynchronous(initalisepacket);
		sendPacketSynchronous(map_i);
		sendPacketSynchronous(map_m);
		sendPacketSynchronous("0|B|2|3|4|7|8"); // TODO: dbutils getammo laserid
		sendPacketSynchronous("0|3|1|2|3"); // TODO: dbutils getrocket rocketid
		sendPacketSynchronous("0|g|a|b,1000,1,10000,C,2,500,U,3,1000,U,5,1000,U|r,100,1,10000,C,2,50000,C,3,500,U"); //SetPriceAMMO
		updateCargo(); //async
		sendPacketSynchronous("0|S|CFG|1"); // TODO: a   l o t   o f   d b   s t u f f
		setResourcePrices();

		if (m_player.isadmin)
			sendPacketSynchronous("0|A|ADM|CLI|1");
		sendPacketSynchronous("0|8");


		generateObjects(m_player.mapid);//async packets
		generatePlayer();//async
		generateCollectables();//async
		generateAliens();//async


		sendEveryone(spawnMe);
		sendEveryone(drones);
		sendEveryone(invis);
		sendEveryone(moveFix);
	}
	catch (boost::bad_lexical_cast& blc)
	{
		warncout << "Bad lexical cast. Kicking User " << m_id << ". Error: " << blc.what() << cendl;
		std::string invalidsess = m_sendpacket.error_invalidSession();
		sendPacketSynchronous(invalidsess);
		disconnectUser(false);
	}
}
void CPlayerHandler::jump(map_t wantedMapID, pos_t dest_x, pos_t dest_y) {
	try {
		/* Cancel attack when jumping*/
		if (m_pbIsAttacking)
		{
			m_pbIsAttacking = false;
		}

		/* Update playerinfo */
		constexpr bool updateLocation = false;
		initPlayerOnJump(updateLocation);
		m_mm->reset(dest_x, dest_y);
		m_player.mapid = wantedMapID;

		/* Switch the session. between leaveSession and joinSession possible i|m_id packets for other players. Nothing important though as the player is immobilized at this moment and cannot receive packets*/
		/* ^ this becomes problematic if the program crashes between leaveSession and joinSession, which is not possible. */
		/* TODO when temp sessions are implemented ^ test this*/
		std::string removeme = m_sendpacket.removeOpponent(m_id);
		sendEveryone(removeme);
		leaveSession();
		joinSession(m_player.mapid);

		/* Build and send the packets */
		std::string ss = m_sendpacket.displayStarSystem();
		std::string initalisepacket = m_sendpacket.init(m_id, m_player.username, m_player.shipid, m_player.speed, m_player.shd,
			m_player.maxshd, m_player.hp, m_player.maxhp, m_player.cargospace, m_player.cargospacemax, dest_x, dest_y, m_player.mapid,
			m_player.factionid, m_player.clanid, m_player.maxbatt, m_player.maxrock, m_player.oState, m_player.premium, m_player.exp,
			m_player.hon, m_player.lvl, m_player.cred, m_player.uri, m_player.jackpot, m_player.rank, m_player.clantag, m_player.gatesAchieved, m_player.useSysfont);
		std::string map_i = m_sendpacket.loadMap(wantedMapID);
		std::string map_m = m_sendpacket.loadMiniMap(wantedMapID);
		std::string updateOres = m_sendpacket.updateOres(m_player.loot.o1_prometium, m_player.loot.o1_endurium, m_player.loot.o1_terbium,
			m_player.loot.o4_xenomit,
			m_player.loot.o2_prometid, m_player.loot.o2_duranium,
			m_player.loot.o3_promerium);
		std::string spawnMe = m_sendpacket.spawnEnemy(m_id, m_player.shipid, m_player.oState, m_player.clantag, m_player.username, dest_x + 410, dest_y + 300, m_player.factionid, m_player.clanid, m_player.rank, false, 1, 3);

		sendPacketSynchronous(ss);
		sendPacketSynchronous(initalisepacket);
		sendPacketSynchronous(map_i);
		sendPacketSynchronous(map_m);
		sendPacketSynchronous(updateOres);
		sendPacketSynchronous("0|B|2|3|4|7|8");
		sendPacketSynchronous("0|3|1|2|3");
		sendPacketSynchronous("0|g|a|b,1000,1,10001,C,2,500,U,3,1000,U,5,1000,U|r,100,1,10000,C,2,50000,C,3,500,U"); //SetPriceAMMO
		sendPacketSynchronous("0|8");
		sendPacketSynchronous("0|A|RCD"); // rocket cooldown reset due to jump interrupt
		setResourcePrices();
		
		
		generateObjects(wantedMapID);
		generatePlayer();
		generateCollectables();
		generateAliens();
		/* Update player properties on map */
		checkForObjectsToInteract();

		dcout << "Jumping - packets to everyone" << cendl;
		sendEveryone(spawnMe);
		sendEveryone("0|n|d|" + boost::lexical_cast<std::string>(m_id) + "|" + m_player.drones);
		if (m_pbIsInvisible)
		{
			//send cloak packet if invisible (not calling cloak() because this is only packet-wise)
			std::string inv = m_sendpacket.cloak(m_id, 1);
			sendEveryone(inv);
		}

		db_updateLocation();

		m_pbIsJumping = false;
	}
	catch (mysql_exception& me)
	{
		warncout << "MySQL Exception thrown. Disconnecting User " << m_id << cendl;
		disconnectUser(true);
	}
	catch (boost::bad_lexical_cast& blc)
	{
		warncout << "Bad lexical cast. Kicking User " << m_id << ". Error: " << blc.what() << cendl;
		disconnectUser(true);
	}
}

void CPlayerHandler::setResourcePrices()
{
	const double honorMultip = 1 + m_player.hon / 500000.0;
	const double resourceMultiplier = boost::algorithm::clamp(honorMultip, 1.0, 2.0);
	std::string pricePackets = m_sendpacket.setOrePrices(
		g_oreinfo[1].getCreditPrice() * resourceMultiplier, g_oreinfo[2].getCreditPrice() * resourceMultiplier, g_oreinfo[3].getCreditPrice() * resourceMultiplier,
		g_oreinfo[11].getCreditPrice() * resourceMultiplier, g_oreinfo[12].getCreditPrice() * resourceMultiplier,
		g_oreinfo[13].getCreditPrice() * resourceMultiplier
	);
	sendPacket(pricePackets);
}

void CPlayerHandler::logoutHandle()
{
	const uint64_t before_logout_sleep_time = m_last_logout_time;
	boost::this_thread::sleep_for(boost::chrono::seconds(m_player.premium ? Constants::Game::LOGOUT_TIME_PREMIUM : Constants::Game::LOGOUT_TIME));
	if (!m_blogout_cancel && m_last_logout_time == before_logout_sleep_time)
	{
		dcout << "LOGGING OUT USER " << m_id << cendl;
		sendPacket("0|l");
	}
}

void CPlayerHandler::onTrade(ore_t oreid, ore_t oreamount)
{
	// GET -> WORK -> SET
	ore_t actualAmount = reduceOre(oreid, std::max(oreamount, (ore_t)0));
	if (actualAmount > 0)
	{
		switch (oreid)
		{
		case 1:
			DBUtil::funcs::removePrometium(m_id, actualAmount);
			break;
		case 2:
			DBUtil::funcs::removeEndurium(m_id, actualAmount);
			break;
		case 3:
			DBUtil::funcs::removeTerbium(m_id, actualAmount);
			break;
		case 11:
			DBUtil::funcs::removePrometid(m_id, actualAmount);
			break;
		case 12:
			DBUtil::funcs::removeDuranium(m_id, actualAmount);
			break;
		case 13:
			DBUtil::funcs::removePromerium(m_id, actualAmount);
			break;
		default:
			return; // OTHER ORE IDS ARE NOT IN RANGE OF SELLING
		}
		const double honorMultip = 1 + m_player.hon / 500000.0;
		const double resourceMultiplier = boost::algorithm::clamp(honorMultip, 1.0, 2.0);
		long long price = credits_t(g_oreinfo[oreid].getCreditPrice() * resourceMultiplier) * actualAmount;
		long long cred = addCredits(price);

		DBUtil::funcs::setCRD(m_id, cred);
		sendPacket(m_sendpacket.receiveLoot("CRE", { price,cred }));
		updateCargo();
	}
}

void CPlayerHandler::db_updateOnLeave()
{
	try {
		DBUtil::funcs::setMap(m_id, m_player.mapid);
		DBUtil::funcs::setPos(m_id, m_mm->get_current_position());
		DBUtil::funcs::setHP(m_id, m_player.hp);
		DBUtil::funcs::setSHD(m_id, m_player.shd);
	}
	catch (mysql_exception& ex)
	{
		warncout << "Could not update Database when user " << fancyPrintName() << " left: " << ex.what() << cendl;
	}
}

dist_t CPlayerHandler::distanceTo(Position_t pos)
{
	Position_t mypos = m_mm->get_current_position();

	pos_t dx = mypos.first - pos.first;
	pos_t dy = mypos.second - pos.second;

	return std::sqrt(dx * dx + dy * dy);
}

dist_t CPlayerHandler::distanceTo(id_t id)
{
	Position_t pos;
	if (id < BEGIN_MOB_IDS)
	{
		handlePtr enemy = m_currentSession->getHandler(id);
		if (enemy)
			pos = enemy->getPos();
		else
			return std::numeric_limits<pos_t>::max();
	}
	else {
		auto mob = m_currentSession->getMob(id);
		if (mob)
			pos = mob->getPosition();
		else
			return std::numeric_limits<pos_t>::max();
	}
	return distanceTo(pos);
}

void CPlayerHandler::updateHealth(damage_t dmg)
{
	sendPacket(m_sendpacket.damageBubbleSelf(m_player.hp, m_player.shd, dmg));
	sendPacket(m_sendpacket.updateHealth(m_player.hp, m_player.maxhp, m_player.shd, m_player.maxshd));
}
void CPlayerHandler::updateShield(damage_t dmg)
{
	//sendPacket(m_sendpacket.damageBubbleSelf(m_player.hp, m_player.shd, dmg));
	sendPacket(m_sendpacket.updateSHD(m_player.shd, m_player.maxshd));
}
void CPlayerHandler::updateHitpoints(damage_t dmg)
{
	//sendPacket(m_sendpacket.damageBubbleSelf(m_player.hp, m_player.shd, dmg));
	sendPacket(m_sendpacket.updateHP(m_player.hp, m_player.maxhp));
}

void CPlayerHandler::updateSpeed(speed_t speed)
{
	m_player.speed = speed;
	sendPacket(m_sendpacket.updateSpeed(speed));
	m_mm->set_speed(speed);
}

void CPlayerHandler::updateCargo()
{
	refreshCargo();
	sendPacket(m_sendpacket.updateOres(m_player.loot.o1_prometium, m_player.loot.o1_endurium, m_player.loot.o1_terbium,
		m_player.loot.o4_xenomit,
		m_player.loot.o2_prometid, m_player.loot.o2_duranium,
		m_player.loot.o3_promerium));
}

void CPlayerHandler::db_updateLocation()
{
	DBUtil::funcs::setMap(m_id, m_player.mapid);
	DBUtil::funcs::setPos(m_id, m_mm->get_current_position());
}

void CPlayerHandler::refreshCargo()
{
	// IRRELEVANT ON "I" PACKET. THE REAL VALUE IS CALCULATED ON "E" PACKET
	m_player.cargospace = DBUtil::funcs::getCargo(m_id);

	m_player.loot.o1_prometium = DBUtil::funcs::getPrometium(m_id);
	m_player.loot.o1_endurium = DBUtil::funcs::getEndurium(m_id);
	m_player.loot.o1_terbium = DBUtil::funcs::getTerbium(m_id);
	m_player.loot.o2_prometid = DBUtil::funcs::getPrometid(m_id);
	m_player.loot.o2_duranium = DBUtil::funcs::getDuranium(m_id);
	m_player.loot.o3_promerium = DBUtil::funcs::getPromerium(m_id);
	m_player.loot.o4_xenomit = DBUtil::funcs::getXenomit(m_id);

	m_player.cargospacemax = g_shipinfo[m_player.shipid].getCargo();
}

void CPlayerHandler::refreshAccount()
{
}

void CPlayerHandler::generateObjects(const map_t mapid) {

	auto ptr = g_allMaps.find(mapid);
	if (ptr == g_allMaps.end())
	{
		return;
	}
	CMap& m = ptr->second;
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
		std::string createdStation = m_sendpacket.createStation(it.stationIDOnMap, 1, it.stationName, it.company, 1,
			it.x, it.y);
		sendPacket(createdStation);
	}
}
void CPlayerHandler::generatePlayer() {
	// dont catch mysql_exception, this function gets called in a try-catch environment
	m_currentSession->lockConnectionsRead();
	for (auto& it : m_currentSession->getAllConnections()) {
		if (it.first != m_id)
		{
			id_t uid = it.second->getID();
			shipid_t shipID = DBUtil::funcs::getShip(uid);
			factionid_t companyID = DBUtil::funcs::getCompany(uid);
			Position_t pos = std::make_pair(it.second->getX() + 410, it.second->getY() + 300);
			bool invisible = it.second->m_pbIsInvisible;
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
			// there should be as short of a time between spawn and invisible
			std::string invisstring = m_sendpacket.cloak(uid, true);


			sendPacket(spawnstring);
			sendPacket("0|n|d|" + boost::lexical_cast<std::string>(uid) + "|" + DBUtil::funcs::getDrones(uid));
			if (invisible)
				sendPacket(invisstring);
		}
	}
	m_currentSession->unlockConnectionsRead();
}
void CPlayerHandler::generateAliens() {

	m_currentSession->lockMobsRead();
	for (auto mob : m_currentSession->getMobs())
	{
		if (mob.second != nullptr)
		{
			mob.second->spawn(m_id);
		}
	}
	m_currentSession->unlockMobsRead();

}
void CPlayerHandler::generateCollectables()
{
	m_currentSession->lockCollectablesRead();
	for (auto coll : m_currentSession->getCollectables())
	{
		if (coll.second != nullptr) //TODO: test if actually neccesary, on all generate functions
		{
			coll.second->spawn(m_id);
		}
	}
	m_currentSession->unlockCollectablesRead();
}
void CPlayerHandler::generatePlayer(id_t id) {
	try {
		auto& enemy = m_currentSession->getHandler(id);
		if (enemy != nullptr)
		{
			id_t uid = enemy->getID();
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
			std::string spawnstring = m_sendpacket.spawnEnemy(uid, shipID, 3, "PO", DBUtil::funcs::getUsername(uid), enemy->getX() + 410, enemy->getY() + 300,
				companyID, clanID, rank, false, 1, 4);

			sendPacket(spawnstring);
			sendPacket("0|n|d|" + boost::lexical_cast<std::string>(uid) + "|" + DBUtil::funcs::getDrones(uid));
		}
	}
	catch (std::out_of_range& oor)
	{
		std::cout << oor.what() << std::endl;
	}
}
void CPlayerHandler::checkForObjectsToInteract() {


	//for the sake of multiple read/writes these two for each loops have to create temporary new "Portals/Stations" instead of iterating through them.
	auto portals = m_currentMap.getPortals();
	auto stations = m_currentMap.getStations();

	m_bCloseToGate = std::any_of(portals.begin(), portals.end(), [&](const CPortal& p) {
		return isInRangeCircle(CPortal::RANGE, p.x, p.y);
		});
	m_bCloseToStation = std::any_of(stations.begin(), stations.end(), [&](const CStation& s) {
		return isInRangeSquare(CStation::RANGE, s.x, s.y);
		});

	auto pos = m_mm->get_current_position();
	m_bInRadiationzone = pos.first > m_currentMap.getWidth() || pos.first < 0 ||
		pos.second > m_currentMap.getHeight() || pos.second < 0;
	//TO-DO add other check, etc.

	std::string packet = m_sendpacket.events(m_bCloseToGate && this->getShieldPreventTime() ? 1 : m_bCloseToStation ? 1 : 0, m_bIsRepairing, m_bCloseToStation, m_bInRadiationzone, m_bCloseToGate, 0,pos.first,pos.second);
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
	return (position.first - objx) * (position.first - objx) + (position.second - objy) * (position.second - objy) < radius * radius;
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
void CPlayerHandler::initPlayerOnLogin()
{
	/* Refresh player information */
	m_pbIsInvisible = DBUtil::funcs::isInvisible(m_id); // O
	m_player.shd = DBUtil::funcs::getSHD(m_id); // O
	m_player.maxshd = DBUtil::funcs::getmaxSHD(m_id); // O
	m_player.hp = DBUtil::funcs::getHP(m_id); // O
	m_player.maxhp = DBUtil::funcs::getmaxHP(m_id); // O
	m_player.oState = 3;// O (see basedamage comment below)
	m_player.basedamage = DBUtil::funcs::getDamage(m_id);// O? (damage should be changable by interaction with the webpage)
	m_player.useSysfont = true;// O

	constexpr bool updateLocation = true;
	initPlayerOnJump(updateLocation);
}
void CPlayerHandler::initPlayerOnJump(bool updateLocation)
{
	m_player.shipid = DBUtil::funcs::getShip(m_id); // A
	m_player.isadmin = DBUtil::funcs::isAdmin(m_id); // A
	m_player.speed = DBUtil::funcs::getSpeed(m_id); // A
	refreshCargo(); // A
	m_player.factionid = DBUtil::funcs::getCompany(m_id);// A
	m_player.clanid = DBUtil::funcs::getUserClanId(m_id);// A
	m_player.clantag = DBUtil::funcs::getClanTag(m_player.clanid);// A
	m_player.maxbatt = g_shipinfo[m_player.shipid].getBaseAmmo();// A
	m_player.maxrock = g_shipinfo[m_player.shipid].getBaseRockets();// A
	m_player.premium = DBUtil::funcs::isPremium(m_id);// A
	m_player.exp = DBUtil::funcs::getEXP(m_id);//A
	m_player.hon = DBUtil::funcs::getHON(m_id);//A
	m_player.lvl = DBUtil::funcs::getLevel(m_id);//A
	m_player.cred = DBUtil::funcs::getCRD(m_id);//A
	m_player.uri = DBUtil::funcs::getURI(m_id);//AA
	m_player.jackpot = DBUtil::funcs::getJP(m_id);// A
	if (m_player.isadmin == false)
	{
		m_player.rank = DBUtil::funcs::getRank(m_id);
	}
	else
	{
		m_player.rank = DBUtil::funcs::getRank(m_id); 
	}
	m_player.gatesAchieved = 3;// A
	m_player.username = DBUtil::funcs::getUsername(m_id);// A
	m_player.drones = DBUtil::funcs::getDrones(m_id);// A
	if (updateLocation)
	{
		Position_t pos;
		m_mm->reset(pos = DBUtil::funcs::getPos(m_id));
		m_player.mapid = DBUtil::funcs::getMap(m_id);// A
	}
	m_mm->set_speed(m_player.speed);
}
void CPlayerHandler::handle_read(const boost::system::error_code& ec, size_t bytes)
{
	switch (ec.value()) {
	case 0:
		break;
	case boost::asio::error::eof:
	case boost::asio::error::connection_reset:
		warncout << "User connection lost: " << m_id << cendl;
		disconnectUser(false);
		return;
	case 1236:
		//disconnected by e.g. attack handler from other player
		dcout << "handle_read err: " << 1236 << cendl;
		return;
	default:
		//dcout << "[Warning] Unknown error \"" << ec.value() << "\" at " << __FUNCTION__ << "(" << __LINE__ << "). Closing connection." << cendl;
		warncout << "Unknown exception message: " << ec.message() << cendl;
		disconnectUser(true);
		return;
	}
	std::istream is(&m_buffer);
	std::getline(is, m_information);

#if PRINT_PACKETS == 1
	dcout << "readPacket: " << m_information<< cendl;
#endif
	handle_read_b(bytes);
}

//due to boost::bind not allowing overloading of bound functions, we must rename this
void CPlayerHandler::handle_read_b(size_t bytes)
{
	m_last_ping_time = getTimeNow();
	try{
		if (bytes > 0) {
			using namespace Constants::Received; // incoming packet headers
			using namespace Constants; // for Constants::Send namespace, as it is not often used
			using boost::lexical_cast;

			fixPacket();// trim, make pretty

			boost::algorithm::split(m_splitpackets, m_information, boost::is_any_of("|"));

			try { //try for bad_lexical_cast globally stopping packet check
				if (m_bLoginSent == false)
				{
					if (packetIs(W_POLICYFILEREQUEST)) {
						warncout << "Received policy-file-request. Something wrong with the PolicyServer?" << cendl;
						//send crossdomain policy constant. This should be removed on official releases. though not needed.
						sendPacket(Send::W_CROSSDOMAINPOLICY.c_str());
					}
					else if (packetIs(LOGIN_REQUEST))
					{
						id_t id = lexical_cast<id_t>(m_splitpackets.at(1));
						std::string sessionID = m_splitpackets.at(2);
						if (authCertification(id, sessionID))
						{
							m_id = id;
							m_sessionId = sessionID;
							bool success = login();
							if (!success)
								return;
						}
						else
						{
							// wrong accout or smth message + kick
							std::string invalidsess = m_sendpacket.error_invalidSession();
							sendPacketSynchronous(invalidsess);
							kickcout << "Session id " << sessionID << " tried to log in with userID " << id << " (Invalid session auth)" << cendl;
							// Shared Pointer is only held on async_read(this function). if function disappears, destructor is called
							return;
						}
					}
				}
				else { // when logged in, user is guaranteed to have m_mm initalized
					if (packetIs(LOGIN_REQUEST)) {
						dcout << "But papa login is already sent" << cendl;
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
							bool levelIsReasonForNoJump = false;
							auto portals = m_currentMap.getPortals();
							auto myPortal = std::find_if(portals.begin(), portals.end(), [&](const CPortal& p)
								{
									return isInRangeCircle(CPortal::RANGE, p.x, p.y);
								});
							if (myPortal != portals.end())
							{
								//PORTAL FOUND

								// compute level of portal and check if user is level
								level_t levelNeeded = 0;
								if (myPortal->faction == m_player.factionid)
								{
									levelNeeded = myPortal->level_for_own_company;
								}
								else
								{
									levelNeeded = myPortal->level_for_enemy_company;
								}

								if (m_player.lvl < levelNeeded)
								{
									//NOT ENOUGH LEVEL
									sendPacket("0|k|" + to_string(levelNeeded));
								}
								else
								{
									//JUMPING!
									m_pbIsJumping = true;
									sendPacket("0|U|" + lexical_cast<std::string>(myPortal->destination_mapID) + "|" + lexical_cast<std::string>(myPortal->portalIDonMap));
									async_func(2000, &CPlayerHandler::jump, myPortal->destination_mapID, myPortal->destination_x, myPortal->destination_y);

								}
							}
							else
							{
								sendMessagePacket("Yo dumbass you can't jump if theres no gate ya idiot");
							}
						}
					}
					else if (packetIs("0")) {
						/* Debug server mirror message */
						if (m_player.isadmin)
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
						updateLogoutTime(); // no shield prevent
						//determining the local and planned position

						/* LOGOUT BUTTON - SECURITY*/
						if (m_splitpackets.at(1) != "NaN" && m_splitpackets.at(2) != "NaN") {
							pos_t px = lexical_cast<pos_t>(m_splitpackets.at(1));
							pos_t py = lexical_cast<pos_t>(m_splitpackets.at(2));

							m_mm->move(px, py);
							Position_t position(m_mm->get_current_position());
							std::string move = m_sendpacket.move(m_id, position.first, position.second, px, py, m_mm->get_time_for_destination());
							sendEveryone(move);
						}
						else
						{
							auto npos = std::string::npos;
							// NaN | NaN | NaN | NaN -> error in system, currently happening when you move to a position on the minimap where youre already heading (rarely, idk why exactly)
							// Nan | Nan | 123 | 123 -> user logout, last position on 123|123
							if (m_splitpackets.at(3).find("Infinity") != npos
								|| m_splitpackets.at(3).find("NaN") != npos
								|| m_splitpackets.at(4).find("Infinity") != npos
								|| m_splitpackets.at(4).find("NaN") != npos) {
								g_filewrite.writemore("User " + to_string(m_id) + " MOVE REQUEST BUG \"" + m_information + "\"", "Client"); //client error
								disconnectUser();
							}

						}
					}
					else if (packetIs(LOGOUT))
					{
						toggleLogoutCancel(false);
						updateLogoutTime();
						async_func(&CPlayerHandler::logoutHandle);
					}
					else if (packetIs(COLLECT_BOX))
					{
						id_t boxID = lexical_cast<id_t>(m_splitpackets.at(1));
						std::shared_ptr<ICollectable> collectable = m_currentSession->getCollectable(boxID);
						if (collectable)
						{
							collectable->collect(m_id);
						}
						else
						{
							//Sends "Box already collected"
							sendPacket(m_sendpacket.receiveLoot("BAH"));
						}
					}
					else if (packetIs(LOGOUT_CANCEL))
					{
						toggleLogoutCancel(true);
					}
					else if (packetIs(LASERSHOOT_START)) {
						id_t uid = 0;
						uid = lexical_cast<id_t>(m_splitpackets.at(1));

						attackID(uid);
					}
					else if (packetIs(ROCKETSHOOT)) {
						//KEEP IN MIND THIS IS NOT CHEAT-PROOF AND I AM TOO LAZY TO CHANGE IT WITH TIMESTAMPS
						id_t uid = 0;

						uid = lexical_cast<id_t>(m_splitpackets.at(1));

						handlePtr enemy = m_currentSession->getHandler(uid);
						if (uid < 12 && m_id >= 12)
						{
							sendPacket("0|A|RCD");
							sendMessagePacket("You can't attack a testing Unit!");
						}
						else if (distanceTo(uid) > g_rocketinfo[m_rockettype].getMaxDistance())
						{
							sendMessagePacket("Ship is too far away.");
							sendPacket("0|A|RCD");
						}
						else if (enemy && enemy->isInPeaceZone())
						{
							sendPacket(m_sendpacket.peaceArea());
							enemy->sendPacket(m_sendpacket.peaceArea());
							sendPacket("0|A|RCD");
						}
						else
						{
							sendPacketAfter(2000, "0|A|RCD");

							//player loses invisibility
							uncloak();

							if (uid < BEGIN_MOB_IDS) // PLAYER
							{
								//PLAYER
								if (checkSelectedID(uid)) {
									std::string shoot;
									std::string bubbleMe;
									std::string bubbleEnemy;
									try {
										damage_t damage = 0;
										if (!enemy->m_pbHasISH)
											damage = m_damageManager.damageRocket(uid, m_rockettype);
										shoot = m_sendpacket.rocketAttack(m_id, uid, m_rockettype, damage); //if damage == 0 hit is false
										bool dead = !makeDamage(damage, enemy);
										bubbleMe = m_sendpacket.damageBubbleSelected(enemy->getHP(), enemy->getSHD(), damage, false);
										sendEveryone(shoot);
										sendPacket(bubbleMe);
										if (dead) //enemy died
										{
											enemy->die();
										}

										updateRepairPrevent();
										enemy->updateNAZPrevent();
										updateLogoutTime();
										enemy->updateRepairPrevent();
										enemy->updateShieldPrevent();
										enemy->updateNAZPrevent();
										enemy->updateLogoutTime();
									}
									catch (std::out_of_range& oor) {}

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
									//if (!mob->m_pbHasISH) later maybe GG
									damage = m_damageManager.damageRocket(uid, m_rockettype);
									shoot = m_sendpacket.rocketAttack(m_id, uid, m_rockettype, damage); //if damage == 0 hit is false
									bool dead = !makeDamage(damage, mob);

									bubbleMe = m_sendpacket.damageBubbleSelected(mob->getShip().hp, mob->getShip().shd, damage, false);
									sendEveryone(shoot);
									sendPacket(bubbleMe);
									if (dead)
									{
										// Later for quest perhaps 
										// onMobDead(monster id and stuff)

										//mob->die();
									}

									updateRepairPrevent();
									updateLogoutTime();

								}

							}
						}
					}
					else if (packetIs(LASERAMMO_CHANGE)) {
						lasertype_t laserid = 0;
						laserid = lexical_cast<lasertype_t>(m_splitpackets.at(1));
						//ELaserColor = color			lasertype_t = ammo
						if (laserid > 5 || laserid < 1)
						{
							g_filewrite.writemore("USERID: " + lexical_cast<std::string>(m_id) + " ILLEGAL ACTVITY! USER CHOSE AMMO TYPE WHICH DOES NOT EXIST!", "Illegal");
							warncout << "WARNING: " << m_id << " used illegal m_lasertype " << laserid << cendl;
							laserid = boost::algorithm::clamp(laserid, 1, 5);
						}
						m_lasertype = laserid;
						if (m_pbIsAttacking) {
							//ELaserColor laserColor = CDamageManager::laserTypeToColor(m_lasertype, m_player.oState);
							//std::string attackLaser = m_sendpacket.laserAttack(m_id, m_selectedOpponent, laserColor);
							//sendEveryone(attackLaser);
						}
					}
					else if (packetIs(ROCKETAMMO_CHANGE)) {
						rockettype_t rocketid = 0;

						rocketid = lexical_cast<rockettype_t>(m_splitpackets.at(1));
						if (rocketid > 3 || rocketid < 1)
						{
							g_filewrite.writemore("USERID: " + lexical_cast<std::string>(m_id) + " ILLEGAL ACTVITY! USER CHOSE ROCKET TYPE WHICH DOES NOT EXIST!", "Illegal");
							warncout << "WARNING: " << m_id << " used illegal m_rockettype " << rocketid << cendl;
							rocketid = boost::algorithm::clamp(rocketid, 1, 3);
						}
						m_rockettype = static_cast<ERocketType>(rocketid);
					}
					else if (packetIs(LASERSHOOT_ABORT)) {
						m_pbIsAttacking = false;
					}
					else if (packetIs(TRADEORE_SEND)) // sent,when arrived at station or drone opened
					{
						this->onSpaceStationTradeRequest();
					}
					else if (packetIs(TRADEORE))
					{
						if (m_bCloseToStation /* || m_extras.hasHM7*/)
							onTrade(lexical_cast<ore_t>(m_splitpackets.at(1)), lexical_cast<ore_t>(m_splitpackets.at(2)));
						else
							sendMessagePacket("You have to be near a company station to sell resources.");
					}
					else if (packetIs(SPECIAL))
					{
						
                                               if (packetIsLevel(SPECIAL_MINE, 1)) {
							//std::string minePacket = Constants::Send::DETONATEMINE + "1/1";
							//sendPacket(minePacket);
					       }

					           else if (packetIsLevel(SPECIAL_SMARTBOMB, 1)) {
							//high priority
							long long SMB_CD_MS = 10000; // TODO <- premium? other stuff?
							if (timeHasPassed(m_smb_next_use))
							{
								m_smb_next_use = getTimeNowDelta(SMB_CD_MS);
								m_pbIsInSMBCooldown = true;

								std::string cooldownPacket = "0|A|CLD|SMB|";
								cooldownPacket += to_string(SMB_CD_MS / 1000);
								sendPacket(cooldownPacket);

								std::string userID_string = lexical_cast<std::string>(m_id);
								sendEveryone("0|n|SMB|" + userID_string);

								async_func(10000, &CPlayerHandler::detonateSMB);



								m_currentSession->lockConnectionsRead();
								for (auto& it : m_currentSession->getAllConnections()) {
									auto& opponentHandle = it.second;
									if (isInRangeCircle(500, opponentHandle->getX(), opponentHandle->getY()) &&
										!(opponentHandle->m_pbHasISH) && opponentHandle->getID() != m_id &&
										opponentHandle->getID() < BEGIN_MOB_IDS) {
										opponentHandle->receiveDamageHP(opponentHandle->getHP() * 0.2);
									}
								}
								m_currentSession->unlockConnectionsRead();
							}
						}
						else if (packetIsLevel(SPECIAL_INSTASHIELD, 1)) {
							long long ISH_CD_MS = 10000; // TODO <- premium? other stuff?
							if (timeHasPassed(m_ish_next_use))
							{
								m_ish_next_use = getTimeNowDelta(ISH_CD_MS);
								m_pbHasISH = true;

								std::string cooldownPacket = "0|A|CLD|ISH|";
								cooldownPacket += to_string(ISH_CD_MS / 1000);
								sendPacket(cooldownPacket);

								std::string userID_string = lexical_cast<std::string>(m_id);
								sendEveryone("0|n|ISH|" + userID_string);
								long long delta = ISH_CD_MS - 3000;
								async_func(3000, &CPlayerHandler::detonateISH, delta);
							}
						}

						else if (packetIsLevel(SPECIAL_REPBOT, 1))
						{
							auto now = getTimeNow();
							if (getTimeNow() - 2500 > m_last_repair_prevention) // 3 seconds cooldown
							{
								if (m_player.hp < m_player.maxhp)
								{
									m_bIsRepairing = true;
								}
								else
								{
									//idk some other predetermined message maybe? actually already happens client side
								}
							}
							else
							{
								//idk some predetermined message maybe?
								sendMessagePacket("Cannot repair right now.");
							}
						}
						else if (packetIsLevel(SPECIAL_CLOAK, 1))
						{
							cloak();
						}
						else if (packetIsLevel(SPECIAL_CHANGECONFIG, 1))
						{
							unsigned int toConfig = lexical_cast<unsigned int>(m_splitpackets.at(2));
							handleConfigChange(toConfig);
						}
					}
					else if (packetIs(SELECT)) {
						id_t new_selectedOpponent = lexical_cast<id_t>(m_splitpackets.at(1));
						if (new_selectedOpponent != m_selectedOpponent)
						{
							//aborterino attackerino
							m_pbIsAttacking = false;
						}

						auto previousTimeStamp = m_selectedTimeStamp;
						selectOpponent(new_selectedOpponent);
						id_t uid = m_selectedOpponent;
						if (uid < BEGIN_MOB_IDS)
						{
							if (checkSelectedID(uid)) {

								handlePtr opponentHandle = nullptr;
								std::string selectShowBars;
								opponentHandle = m_currentSession->getHandler(uid);
								if (opponentHandle)
								{
									health_t normal_hp_opp = 100;
									health_t max_hp_opp = 100;
									shield_t normal_shd_opp = 100;
									shield_t max_shd_opp = 100;
									normal_hp_opp = opponentHandle->getHP();
									max_hp_opp = opponentHandle->getMaxHP();
									normal_shd_opp = opponentHandle->getSHD();
									max_shd_opp = opponentHandle->getMaxSHD();

									selectShowBars = m_sendpacket.showBars(normal_hp_opp, max_hp_opp, normal_shd_opp, max_shd_opp);
									sendPacket(selectShowBars);
								}

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
						if ((previousTimeStamp + 500 > m_selectedTimeStamp) && // 500 ms time span to trigger attack
							(m_previousSelectedOpponent == m_selectedOpponent) &&
							(m_pbIsAttacking == false) // actually already done inside the function but this keeps some assembly lines off if user just spams lmb
							)
						{
							attackID(m_selectedOpponent);
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
								if (m)
									m->spawn(m_id); //send spawn packets
							}
							else
							{
								try {
									generatePlayer(init_id);
								}
								catch (mysql_exception& me)
								{
									warncout << fancyPrintName() << ": Can't generatePlayer with id " << init_id << ". MySQL Exception: " << me.what() << cendl;
								}
								catch (boost::bad_lexical_cast& me)
								{
									warncout << fancyPrintName() << ": Can't i|" << init_id << "! Generate Player with ID not successfull: " << me.what() << cendl;
								}
								catch (std::exception& me)
								{
									warncout << fancyPrintName() << ": Can't i|" << init_id << "! Generate Player with ID not successfull: " << me.what() << cendl;
								}
							}
						}
					}
	#ifdef DBG_PACKETCLI
					else if (m_player.isadmin)
					{
						if (packetIs("DBG_PACKETTEST")) {
							sendMessagePacket("Currently no packet test ongoing. :)");
						}
						else if (packetIs("DBG_PACKETTEST2")) {
							sendMessagePacket("Currently no 2nd packet test ongoing. :)");
						}
						else if (packetIs("DBG_INTERRUPT_THREADS")) {
							m_workerThreads.stopAllThreads();
						}
						else if (packetIs("DBG_ADDHP")) {
							health_t hp = lexical_cast<health_t>(m_splitpackets.at(1));
							addHP(hp);
						}
						else if (packetIs("DBG_OREIDHERE")) {
							const Position_t p = getPos();
							static id_t oreID = 5000;
							bool ub = false;
							try {
								ub = lexical_cast<int>(m_splitpackets.at(2));
							}
							catch (std::out_of_range oor)
							{
							}
							std::string str = "Ore id: ";
							str += to_string(oreID);
							sendMessagePacket(str);
							sendPacket(m_sendpacket.createOre(oreID++, lexical_cast<ore_t>(m_splitpackets.at(1)), p.first, p.second, ub));

						}
						else if (packetIs("DBG_BOXIDHERE")) {
							const Position_t p = getPos();
							static id_t oreID = 50000;
							std::string str = "Box id: ";
							str += to_string(oreID);
							sendMessagePacket(str);
							sendPacket(m_sendpacket.createLoot(oreID++, lexical_cast<ore_t>(m_splitpackets.at(1)), p.first, p.second));
						}
						else if (packetIs("DBG_SPAWNRESOURCE")) {
							CMobWrapper::Loot loot{ 0 };
							try {
								loot.o1_prometium = lexical_cast<ore_t>(m_splitpackets.at(1));
								loot.o1_endurium = lexical_cast<ore_t>(m_splitpackets.at(2));
								loot.o1_terbium = lexical_cast<ore_t>(m_splitpackets.at(3));
								loot.o2_prometid = lexical_cast<ore_t>(m_splitpackets.at(4));
								loot.o2_duranium = lexical_cast<ore_t>(m_splitpackets.at(5));
								loot.o3_promerium = lexical_cast<ore_t>(m_splitpackets.at(6));
								loot.o4_xenomit = lexical_cast<ore_t>(m_splitpackets.at(7));
							}
							catch (const std::out_of_range& oor)
							{

							}
							const Position_t pos = getPos();
							//what? you want to use the emplace parameter instead of a double allocation? pfft bye
							std::shared_ptr<CResourceBox> res(new CResourceBox(loot, m_currentSession->generateNewCollectableId(), pos.first, pos.second, *m_currentSession, m_id));
							m_currentSession->addCollectable(res);

						}
						else if (packetIs("DBG_SHIP")) {
							DBUtil::funcs::setShip(m_id, lexical_cast<shipid_t>(m_splitpackets.at(1)));
							m_socket.close();
						}
						else if (packetIs("DBG_YEAHBOXES")) {
							for (int i = 0; i < 2000; i++)
							{
								m_currentSession->addCollectable(CBonusBox::generateNewBonusBox(m_currentSession->generateNewCollectableId(), m_currentSession->getMapId(), *m_currentSession));
							}
						}
						else if (packetIs("DBG_GIVE")) {
							std::string what = m_splitpackets.at(1);
							unsigned long long param = lexical_cast<unsigned long long>(m_splitpackets.at(2));

							//copy pate from mob this is a debug function nobody cares
							struct {
								long long credits_loot = 0; //loot
								long long credits_total = 0; //total
								long long uri_loot = 0;
								long long uri_total = 0;
								long long honor_loot = 0;
								long long honor_total = 0;
								long long experience_loot = 0;
								long long experience_total = 0;
							} w;

							w.credits_total = m_player.cred;
							w.uri_total = m_player.uri;
							w.experience_total = m_player.exp;
							w.honor_total = m_player.hon;
							if (what == "CR")
							{
								w.credits_loot = param;
								w.credits_total = addCredits(w.credits_loot);
							}
							else if (what == "URI")
							{
								w.uri_loot = param;
								w.uri_total = addUri(w.uri_loot);
							}
							else if (what == "EP")
							{
								w.experience_loot = param;
								w.experience_total = addEP(w.experience_loot);
							}
							else if (what == "HON")
							{

								w.honor_loot = param;
								w.honor_total = addHonor(w.honor_loot);
							}

							try {
								std::string rawQuery =
									"UPDATE cuentas "
									"SET creditos = creditos + " + to_string(w.credits_loot) + ","
									"uridium = uridium + " + to_string(w.uri_loot) + ","
									"experiencia = experiencia + " + to_string(w.experience_loot) + ","
									"honor = honor + " + to_string(w.honor_loot) + " "
									"WHERE id = " + to_string(m_id);
								g_database_update.queryUpdateRaw(rawQuery);
								if (w.credits_loot)
									sendPacket(m_sendpacket.receiveLoot("CRE", { w.credits_loot,w.credits_total }));
								if (w.uri_loot)
									sendPacket(m_sendpacket.receiveLoot("URI", { w.uri_loot,w.uri_total }));
								if (w.honor_loot)
									sendPacket(m_sendpacket.receiveLoot("HON", { w.honor_loot,w.honor_total }));
								if (w.experience_loot)
									sendPacket(m_sendpacket.receiveLoot("EP", { w.experience_loot,w.experience_total,(long long)getLevel() }));
							}
							catch (mysql_exception& me)
							{
								warncout << "User " << m_id << " mysql_exception: " << me.what() << cendl;
								disconnectUser();
							}

						}
						else if (packetIs("DBG_HITCHANCE"))
						{
							m_damageManager.updateHitChance(lexical_cast<int>(m_splitpackets.at(1)));
						}
						else if (packetIs("DBG_KICK"))
						{
							id_t to = 0;
							try {
								to = lexical_cast<id_t>(m_splitpackets.at(1));
							}
							catch (std::out_of_range& ex) {}
							if (to == 0)
							{
								sendEveryone(m_sendpacket.kill(m_id));
								disconnectUser();
							}
							else
							{
								auto user = m_currentSession->getHandler(to);
								if (user)
								{
									sendEveryone(m_sendpacket.kill(to));

									user->die();
								}
							}
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
									mob->generateRandomWaitingTime(getTimeNow(), 10000, 10000);
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
									mob->generateRandomWaitingTime(getTimeNow(), 10000, 10000);
								}
							}
						}
						else if (packetIs("DBG_MOVE"))
						{
							/*
								UNSAFE METHOD! USE ONLY FOR DEBUGGING PURPOSE. IT CAN POTENTIALLY OVERRIDE
								OTHER ORE IDS
								*/
							pos_t right = lexical_cast<pos_t>(m_splitpackets.at(1));
							pos_t down = lexical_cast<pos_t>(m_splitpackets.at(2));
							if (m_selectedOpponent >= BEGIN_MOB_IDS)
							{
								std::shared_ptr<CMob> mob = m_currentSession->getMob(m_selectedOpponent);
								if (mob != nullptr)
								{
									Position_t pos = mob->getPosition();
									mob->move(pos.first + right, pos.second + down);
									mob->generateRandomWaitingTime(getTimeNow(), 10000, 10000);
									std::thread([&](std::shared_ptr<CMob> mob) {
										int randomInt = random<int>();
										Position_t mobPos = mob->getPosition();
										while (sendPacket(m_sendpacket.createOre(randomInt++, 3, mobPos.first, mobPos.second)) && sendPacket(m_sendpacket.removeOre(randomInt - 2)))
										{
											std::this_thread::sleep_for(std::chrono::milliseconds(33));
											Position_t newpos = mob->getPosition();
											if (mobPos.first == newpos.first && mobPos.second == newpos.second)
											{
												break;
											}
											mobPos = newpos;
										}

										}, mob).detach(); //lets not even talk about a floating thread
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
							catch (const std::out_of_range& oor) {
								dest_x = m_mm->get_current_position_x();
								dest_y = m_mm->get_current_position_y();
							}
							catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
							catch (const boost::exception& e) { std::cerr << BOOST_CERR_OUTPUT(e) << std::endl; }
							dcout << m_id << " jumping to " << wantedMapID << cendl;
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
								if (mob_shipID < 71)
								{
									mob_shipID = 71;
								}
								else if (mob_shipID > 90)
								{
									mob_shipID = 90;
								}

								/* Mob Name*/
								mob_name = "-=[ " + m_splitpackets.at(2) + " ]=- (ID " + to_string(mob_shipID) + ")";


								/* Mob Pos */
								/*x*/
								try {
									mob_x += lexical_cast<pos_t>(m_splitpackets.at(3));
									/*y*/
									mob_y += lexical_cast<pos_t>(m_splitpackets.at(4));
								}
								catch (std::out_of_range& ex)
								{
									mob_x += getX() + 1;
									mob_y += getY() + 1;
								}
							}
							catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
							catch (const boost::exception& e) { std::cerr << BOOST_CERR_OUTPUT(e) << std::endl; }


							std::string spawnmobstring = m_sendpacket.spawnEnemy(mobID++, mob_shipID, 0, "", mob_name, mob_x, mob_y, 0, 0, 0, false, 0, 0, false);
							dcout << spawnmobstring << cendl;
							sendPacket(spawnmobstring);
						}
						else if (packetIs("DBG_AI_BOOM"))
						{
							CSession::NpcContainer_t allNpcs = m_currentSession->getMobs();
							std::vector<std::shared_ptr<CMob>> mobs;
							for (auto mob : allNpcs)
							{
								if (mob.second)
								{
									mobs.push_back(mob.second);
								}
							}
							for (auto mob : mobs)
							{
								mob->die();
							}
						}
						else if (packetIs("DBG_SPEED"))
						{
							speed_t mynewspeed = lexical_cast<speed_t>(m_splitpackets.at(1));
							updateSpeed(mynewspeed);
							std::string msg = "Updated speed to ";
							msg += mynewspeed;
							sendMessagePacket(msg);
						}
						else if (packetIs("DBG_AI_FIRE"))
						{
							m_currentSession->lockMobsRead();
							for (auto mobpair : m_currentSession->getMobs())
							{
								std::shared_ptr<CMob>& mob = mobpair.second;
								if (mob)
								{
									id_t id = m_id;
									try {
										id = lexical_cast<id_t>(m_splitpackets.at(1));
									}
									catch (const std::out_of_range& ex) {}
									mob->attack(id);
									handlePtr user = m_currentSession->getHandler(id);
									if (user != nullptr)
										user->receiveDamagePure(mob->getShip().dmg);
									mob->abort();
								}
							}
							m_currentSession->unlockMobsRead();
						}
						else if (packetIs("DBG_THROWMYSQL"))
						{
							sendPacket(m_sendpacket.error_invalidSession());
							throw mysql_exception("MYSQL EXCEPTION DBG");
						}
						else if (packetIs("DBG_HESOYAM"))
						{
							m_player.hp = m_player.maxhp * 2; //YEAH BOI NANO HP
							m_player.shd = m_player.maxshd;
							m_player.basedamage = 50000;
							sendPacket(m_sendpacket.updateHP(m_player.hp, m_player.maxhp));
							sendPacket(m_sendpacket.updateSHD(m_player.shd, m_player.maxshd));
							sendMessagePacket("HESOYAM");
						}
						else if (packetIs("DBG_DAMAGE"))
						{
							m_player.basedamage = lexical_cast<id_t>(m_splitpackets.at(1));
							sendMessagePacket("Set damage!");
						}
						/*
						else if (packetIs("DBG_AI_M2"))
						{
							DBG_SIMULATE_AI_MOVE();
						}*/
						else if (packetIs("DBG_AI_MAGNET"))
						{
							const dist_t distance = 250;
							auto timeNow = getTimeNow();
							for (auto mobpair : m_currentSession->getMobs())
							{
								std::shared_ptr<CMob>& mob = mobpair.second;
								if (mob)
								{
									unsigned int deg = random<uint32_t>(360);
									auto pos = mob->getPosition();
									auto mypos = m_mm->get_current_position();
									decltype(pos) newmobpos = std::make_pair(mypos.first, mypos.second);
									//x
									double degree = deg * M_PI / 180.0;
									newmobpos.first += distance * std::cos(degree);
									newmobpos.second += distance * std::sin(degree);
									mob->move(newmobpos.first, newmobpos.second);
									mob->generateRandomWaitingTime(timeNow, 10000, 50000);
								}
							}

						}
						else if (packetIs("DBG_TO_SELECTED"))
						{
							auto& selectedHandle = m_currentSession->getHandler(m_selectedOpponent);
							if (selectedHandle)
							{
								selectedHandle->sendPacket(m_information);
							}
						}
						else if (packetIs("FLUSH_THREADS"))
						{
							suspendThreads();
						}
					} //elseif-admin
	#endif
				} //end packet check
			}
			catch (const std::out_of_range& oor)
			{
				std::stringstream ss;
				ss << "Unexpected out_of_range in " << __FUNCTION__ << " on packet \"" << m_information << "\" for User " << fancyPrintName() << " with error message: " << oor.what();
				std::string str = ss.str();
				errorcout << str << cendl;
				g_filewrite.writemore(str,"ERROR");
			}
			catch (const boost::bad_lexical_cast& blc)
			{
				std::stringstream ss;
				ss << "Unexpected bad_lexical_cast in " << __FUNCTION__ << " on packet \"" << m_information << "\" for User " << fancyPrintName();
				std::string str = ss.str();
				errorcout << str << cendl;
				g_filewrite.writemore(str,"ERROR");
			}
		}//END OF IF BYTES > 0
		readData();
	}
	catch (const mysql_exception& me)
	{
		std::stringstream ss;
		ss << "Unexpected mysql_exception in " << __FUNCTION__ << " on packet \"" << m_information << "\" for User " << fancyPrintName() << " with error message: " << me.what();
		std::string str = ss.str();
		errorcout << str << cendl;
		g_filewrite.writemore(str, "ERROR");
		disconnectUser(true);
	}
}

void CPlayerHandler::onSpaceStationTradeRequest()
{
	// This gets handled on login and on every honor update. Otherwise trade drones have a delay outside of space stations
}

void CPlayerHandler::savePosToDB()
{

}

void CPlayerHandler::sendMessagePacket(const std::string& text)
{
	sendPacket(m_sendpacket.sendMessage(text));
}

void CPlayerHandler::sendEveryone(const std::string& packet) {
	//dcout << "sendEveryone: " << packet << cendl;
	m_currentSession->sendEveryone(packet);
}

void CPlayerHandler::sendEveryoneButMe(const std::string& packet)
{
	//or session id? both work, 
	// session id is securer and safer, 
	// userID is more surely the user doesnt get it
	m_currentSession->sendEveryoneBut(packet, m_id);
}

void CPlayerHandler::leaveSession()
{
	m_currentSession->leaveSession(m_id);
}

void CPlayerHandler::joinSession(map_t mapid)
{
	g_sessionsManager.joinSession(shared_from_this(), m_id, m_player.mapid);
	m_currentSession = &g_sessionsManager.getAnySession(m_player.mapid);
}

damage_t CPlayerHandler::receiveDamagePure(damage_t dmg)
{
	updateRepairPrevent();
	updateShieldPrevent();
	updateNAZPrevent();
	updateLogoutTime();
	double SHIELDPERCENTAGE = 0.8;
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
		return -dmg; //ded
	}
	else {
		m_player.hp = health_post_damage;
		updateHealth(dmg);
		return dmg; //no ded
	}

}

damage_t CPlayerHandler::receiveDamageHP(damage_t dmg) {
	updateRepairPrevent();
	updateShieldPrevent();
	updateNAZPrevent();
	updateLogoutTime();
	damage_t realdmg = dmg;
	health_t hp = m_player.hp - dmg;

	// to ensure no overflow happens
	if (hp <= 0) {
		realdmg += hp; // 5000 DMG 4000 HP -> -1000 thus 5000 + (-1000) = 4000
		m_player.hp = 0;
		return -dmg;
	}
	else
	{
		m_player.hp = hp;
	}

	updateHitpoints(realdmg);
	return realdmg;
}

damage_t CPlayerHandler::receiveDamageSHD(damage_t dmg) {
	updateRepairPrevent();
	updateShieldPrevent();
	updateNAZPrevent();
	updateLogoutTime();
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

	updateShield(realdmg);
	return realdmg;
}

// TODO. php session id has to hold userID and then have to be the same as here.
bool CPlayerHandler::authCertification(id_t id, std::string sessionID)
{
	return true;
}

void CPlayerHandler::die()
{
	try {
		sendEveryone(m_sendpacket.kill(m_id));
		map_t respawnMap = 1;
		switch (m_player.factionid)
		{
		case 1:
			respawnMap = 1;
			break;
		case 2:
			respawnMap = 5;
			break;
		case 3:
			respawnMap = 9;
			break;
		default:
			g_filewrite.writemore("UserId " + to_string(m_id) + " has factionID not normal (" + to_string(m_player.factionid) + ")", "DIE");
		}
		if (m_currentSession->getMapId() >= 16 && m_currentSession->getMapId() <= 29 && m_player.lvl >= 12)
		{
			respawnMap += 19;
		}
		assert(g_allMaps[respawnMap].getStations().size() != 0);
		pos_t x = g_allMaps[respawnMap].getStations().at(0).x;
		pos_t y = g_allMaps[respawnMap].getStations().at(0).y;

		dcout << "Station at " << x << "/" << y << cendl;
		//TODO: is this safe to use? probably a few ns where this MIGHT be risky. TODO!!
		m_player.hp = 1000;
		m_player.shd = 0;
		m_player.mapid = respawnMap;
		m_mm->reset(x, y);
		db_updateOnLeave();
		disconnectUser(false);
	}
	catch (boost::exception_ptr& e) {
		std::cerr << BOOST_CERR_OUTPUT(e) << std::endl;
	}
	catch (std::exception& e) {
		//std::cerr << STD_CERR_OUTPUT(e) << std::endl;
	}
}
//should happen at least at the end of a users time u know this joins and stuff
void CPlayerHandler::suspendThreads()
{
	m_workerThreads.stopAllThreads();

	/*for (auto& thread : m_asyncThreads)
	{
#define noSUSPEND_ASYNC_THREAD
#ifdef SUSPEND_ASYNC_THREAD
		//doesnt delete shared_ptrs in threads so njiet spasibo
		//TerminateThread(thread->native_handle(), 0);

		SuspendThread(thread->native_handle());
#else
		if (boost::this_thread::get_id() != thread->get_id())
		{
			dcout << "interrupting " << thread->get_id();
			thread->interrupt();
		}
#endif
		delete thread;
		thread = nullptr;
	}
	m_asyncThreads.clear();*/
	// hm
	// m_socket.close();
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

std::string CPlayerHandler::fancyPrintName()
{
	std::stringstream ss;
	ss << "<" << m_player.username << "|" << m_id << ">";
	return ss.str();
}

shield_t CPlayerHandler::addSHD(shield_t shd)
{
	//add shd to current player
	shield_t addShd = shd;
	shield_t p_shd = m_player.shd;
	if ((m_player.shd += addShd) > m_player.maxshd)
	{
		addShd = addShd - (m_player.shd - m_player.maxshd);
		m_player.shd = m_player.maxshd;

	}
	if (p_shd != m_player.shd)
	{
		updateShield(addShd); // perhaps remove parameter its dumb
	}
	return addShd;
}
health_t CPlayerHandler::addHP(health_t hp)
{
	//add shd to current player
	shield_t addHp = hp;
	shield_t p_hp = m_player.hp;
	if ((m_player.hp += addHp) > m_player.maxhp)
	{
		addHp = addHp - (m_player.hp - m_player.maxhp);
		m_player.hp = m_player.maxhp;

	}
	if (p_hp != m_player.hp)
	{
		updateHitpoints(addHp); // perhaps remove parameter its dumb
	}
	return addHp;
}

template<class callable, class ...arguments>
void CPlayerHandler::async_func(callable&& func, arguments&& ... args)
{
	auto f = boost::bind(func, shared_from_this(), args...);
	m_workerThreads.add(f);
}
template<class callable, class ...arguments>
void CPlayerHandler::async_func(int ms, callable&& func, arguments&& ... args)
{
	auto f = boost::bind(func, shared_from_this(), args...);
	auto lambda = [f, ms]() {
		boost::this_thread::sleep_for(boost::chrono::milliseconds(ms));
		f();
	};
	m_workerThreads.add(lambda);
}
