#include "PlayerHandler.h"

#define DBG_PACKETCLI
DBGetter database_get("olddorps", "usuario", "cuentas", 0);
std::map<int, SessionsManager> CPlayerHandler::allInSession;


///////////////////////
//////////////////////
////////////////////
/////////////////////
//////////////////////



auto CPlayerHandler::createConnection(boost::asio::io_service & io_service, unsigned short port) -> CPlayerHandler::tcppointer
{
	CPlayerHandler *newhandle = new CPlayerHandler(io_service, port, allInSession[0]);
	return tcppointer(newhandle);
}

tcp_t::socket & CPlayerHandler::getSocket()
{
	return socket_;
}

void CPlayerHandler::start()
{
	try {
		currentSession.joinSession(shared_from_this());
		readData();
	}
	catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
	catch (const boost::exception& e) { std::cerr << BOOST_CERR_OUTPUT(e) << std::endl; }
}

void CPlayerHandler::readData() {
	boost::asio::async_read_until(socket_, buffer, "\n",
		boost::bind(&CPlayerHandler::handle_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}
void CPlayerHandler::sendPacket(std::string str) {
	std::string packet = str + (char)0; // string + nullterminator
	boost::asio::streambuf sendbuffer;
	std::ostream is(&sendbuffer);
	is << packet;
	boost::asio::async_write(socket_,
		sendbuffer.data(),
		boost::bind(&CPlayerHandler::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}
void CPlayerHandler::sendPacketAfter(int ms, std::string str) {
	asyncThreads .push_back(
		async_func(ms,&CPlayerHandler::sendPacket, str));
}

//packet sending handler
void CPlayerHandler::handle_write(const boost::system::error_code & ec, size_t bytes) { }

void CPlayerHandler::disconnectUser() {
	std::cout << "[INFO] User " << id << " disconnected" << std::endl;
	socket_.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both);
	socket_.close();
	currentSession.leaveSession(shared_from_this());
}

bool CPlayerHandler::packetStartsWith(std::string str) {
	return boost::algorithm::starts_with(information, str);
}

bool CPlayerHandler::makeDamage(damage_t dmg, id_t from, handlePtrIt it) {
	//doing this pointer seemed to have cured this functions problems
	if (it != currentSession.getAllConnections().end()) {
		it->get()->receiveDamagePure(dmg, from);
		return true;
	}
	return false;
}


void CPlayerHandler::handleAttackRequest(id_t uid)
{
	//it indeed should be running flawless unless the thread terminates out of nowhere and the variable isnt changed
	//UID instead of selectedUser since mid-thread program could change the variable and thus if(uid < 120000) is true and afterwards the player could've changed to a mob
	if (!isAttackThreadRunning) {
		isAttackThreadRunning = true;
		asyncThreads.push_back(async_func(&CPlayerHandler::handleAttack,uid));
	}

}
void CPlayerHandler::handleAttack(id_t uid) noexcept {
	//change this when dynamic amm
	const std::string shoot = sendpacket.laserAttack(id, uid, 3);
	while (isAttacking) {
		// PLAYER
		if (uid < 120000) {
			damage_t damage = 0;
			std::string bubbleMe;
			std::string bubbleEnemy;
			handlePtrIt opponentHandle = currentSession.getHandler(uid);

			if (opponentHandle != currentSession.getAllConnections().end()) {
				if (opponentHandle->get()->getMapId() != getMapId())
					break;
				if (!opponentHandle->get()->hasISH)
					damage = damageManager.damageLaser(uid, 1, 20000, false);

				bubbleMe = sendpacket.damageBubbleSelected(opponentHandle->get()->getHP(), opponentHandle->get()->getSHD(), damage, false);
				bubbleEnemy = sendpacket.damageBubbleSelf(opponentHandle->get()->getHP(), opponentHandle->get()->getSHD(), damage);
			}
			else {
				/*//to at least keep the visuals if there is no one of that ID
				bubbleMe = sendpacket.damageBubbleSelected(1, 1, damage, false);
				bubbleEnemy = sendpacket.damageBubbleSelf(1, 1, damage);*/
				// abort attack
				break;
			}
			if (!makeDamage(damage, id, opponentHandle)) {
				break;
			}
			sendPacket(bubbleMe);
			currentSession.sendTo(bubbleEnemy, selectedOpponent);
		}
		// ALIEN
		else {

		}
		sendEveryone(shoot);
		//before we loop again
		if(isAttacking)
			std::this_thread::sleep_for(std::chrono::seconds(1));
	} //end of while

	std::string abort = sendpacket.laserAbort(id, uid);
	sendEveryone(abort);
	isAttackThreadRunning = false;

}

void CPlayerHandler::detonateSMB()
{
	isInSMBCooldown = false; 
	sendPacket("0|A|CLR|SMB");
}

void CPlayerHandler::detonateISH()
{
	hasISH = false;
	// After 10 secs: ISH item available again
	std::this_thread::sleep_for(std::chrono::seconds(7));
	sendPacket("0|A|CLR|ISH");
}

map_t CPlayerHandler::getUserIDMapId(id_t id) {
	// returns the MAP ID of the user with the id "id"
	for (auto& it : allInSession) {
		for (auto& conns : it.second.getAllConnections()) {
			if (conns.get()->getID() == id)
				return conns.get()->getMapId();
		}
	}
	return 0;
}
bool CPlayerHandler::checkSelectedID(id_t id)
{
	//after a jump the id resets -> check if the selectedOpponent equals the attacker (f.e) and also the id must be NOT 0
	if (selectedOpponent == id && selectedOpponent != 0) {
		int currentMapID = currentMap.getMapId();
		int opponentMapID = getUserIDMapId(id);
		return opponentMapID == currentMapID && opponentMapID != 0; // 0 -> not found
	};
	return false;
}

void CPlayerHandler::logInPackets() {
	try {
		//preparing for jump
		player.shipid = DBUtil::funcs::getShip(id);
		player.speed = DBUtil::funcs::getSpeed(id);
		player.shd = DBUtil::funcs::getSHD(id);
		player.maxshd = DBUtil::funcs::getmaxSHD(id);
		player.hp = DBUtil::funcs::getHP(id);
		player.maxhp = DBUtil::funcs::getmaxHP(id);
		player.cargospace = 100;
		player.cargospacemax = 500;
		player.mapid = DBUtil::funcs::getMap(id);
		player.fractionid = DBUtil::funcs::getCompany(id);
		player.clanid = 220;
		player.maxbatt = 20000;
		player.maxrock = 400;
		player.oState = 3;
		player.premium = true;
		player.exp = 5000120;
		player.hon = 400100;
		player.lvl = 42;
		player.cred = 1337420;
		player.uri = 69420;
		player.jackpot = 500.20f;
		player.rank = DBUtil::funcs::getRank(id);
		player.clantag = "PO";
		player.gatesAchieved = 2;
		player.useSysfont = true;
		player.username = DBUtil::funcs::getUsername(id);
		player.drones = DBUtil::funcs::getDrones(id);



		Position_t pos = DBUtil::funcs::getPos(id);
		/*
		IN REMINDER WHAT COST ME 4 DAYS TO FIND; A NEW REWRITTEN DBUTIL THAT RETURNS A PAIR INSTEAD OF A STRING
		AND I'VE LITTERALY GAVE UP MULTIPLE TIMES TO FIND THIS

		RIP: split(std::vector<std::string>,Position_t,...) 2016-2016 u'll not be missed:
											    ^
		boost::algorithm::split(pos, DBUtil::funcs::getPos(id), boost::is_any_of("|"));
		*/

		mm.reset(new MovementManager(pos.first, pos.second, player.speed));
		pos_t x = mm->get_current_position_x();
		pos_t y = mm->get_current_position_y();

		std::string initalisepacket = sendpacket.init(id, player.username, player.shipid, player.speed, player.shd, player.maxshd, player.hp,
			player.maxhp, player.cargospace, player.cargospacemax, x, y, player.mapid, player.fractionid, player.clanid, player.maxbatt,
			player.maxrock, player.oState, player.premium, player.exp, player.hon, player.lvl, player.cred, player.uri, player.jackpot, player.rank,
			player.clantag, player.gatesAchieved, player.useSysfont);
		std::string map_i = sendpacket.loadMap(player.mapid);
		std::string map_m = sendpacket.loadMiniMap(player.mapid);


		allInSession[0].leaveSession(shared_from_this());
		allInSession[player.mapid].joinSession(shared_from_this());
		currentSession = allInSession[player.mapid];

		sendPacket("0|A|SET|1|1|1|1|1|1|1|1|1|1|1|1|0|1|1|1|1|1|1|1|1|0|0|0|0");
		sendPacket("0|u|1");
		sendPacket("0|d|1");
		sendPacket(initalisepacket);
		sendPacket(map_i);
		sendPacket(map_m);
		sendPacket("0|B|2|3|4|7|8");
		sendPacket("0|3|1|2|3");
		sendPacket("0|S|CFG|1");
		sendPacket("0|A|ADM|CLI|1");
		sendPacket("0|8");

		lasertype = LaserType::BLUE;
		rockettype = RocketType::R310;

		generateObjects(player.mapid);
		generatePlayer();
		checkForObjectsToInteract();

		std::string spawnMe = sendpacket.spawnEnemy(id, player.shipid, 3, player.clantag, player.username, x + 410, y + 300, player.fractionid, 220, 20, false, 1, 4);
	
		sendEveryone(spawnMe);
		sendEveryone("0|n|d|" + boost::lexical_cast<std::string>(id) + "|" + player.drones);
	}
	catch (sql::SQLException ex) {
		std::cerr << "[err] Error connecting to the database: " << ex.what() << std::endl;
	}
}
void CPlayerHandler::jump(map_t wantedMapID, pos_t dest_x, pos_t dest_y) {
	mm->reset(dest_x, dest_y);
	try {
		//in case of update these variables will be kept 
		//TODO: maybe keep more 
		player.shipid = DBUtil::funcs::getShip(id);
		player.speed = DBUtil::funcs::getSpeed(id);
		player.mapid = wantedMapID;
		player.fractionid = DBUtil::funcs::getCompany(id);
		player.clanid = 220;
		player.drones = DBUtil::funcs::getDrones(id);
		player.username = DBUtil::funcs::getUsername(id);

		std::string removeme = sendpacket.removeOpponent(id);
		std::string ss = sendpacket.displayStarSystem();
		std::string initalisepacket = sendpacket.init(id, player.username, player.shipid, player.speed, player.shd,
			player.maxshd, player.hp, player.maxhp, player.cargospace, player.cargospacemax, dest_x, dest_y, player.mapid,
			player.fractionid, player.clanid, player.maxbatt, player.maxrock, player.oState, player.premium, player.exp,
			player.hon, player.lvl, player.cred, player.uri, player.jackpot, player.rank, player.clantag, player.gatesAchieved, player.useSysfont);
		std::string map_i = sendpacket.loadMap(wantedMapID);
		std::string map_m = sendpacket.loadMiniMap(wantedMapID);


		sendEveryone(removeme);
		allInSession[currentMap.getMapId()].leaveSession(shared_from_this());
		allInSession[wantedMapID].joinSession(shared_from_this());
		currentSession = allInSession[wantedMapID];
		sendPacket(ss);
		sendPacket(initalisepacket);
		sendPacket(map_i);
		sendPacket(map_m);
		sendPacket("0|B|2|3|4|7|8");
		sendPacket("0|3|1|2|3");
		sendPacket("0|8");

		generateObjects(wantedMapID);
		generatePlayer();
		checkForObjectsToInteract();

		std::string spawnMe = sendpacket.spawnEnemy(id, player.shipid, 3, "PO", player.username, dest_x + 410, dest_y + 300, player.fractionid, 220, 20, false, 1, 3);
		sendEveryone(spawnMe);
		sendEveryone("0|n|d|" + boost::lexical_cast<std::string>(id) + "|" + player.drones);
	}
	catch (sql::SQLException ex) {
		std::cerr << "Error connecting to the database: " << ex.what() << std::endl;
	}
	isJumping = false;
}

void CPlayerHandler::updateHealth()
{
	std::string hp_s = boost::lexical_cast<std::string>(player.hp);
	std::string hpm_s = boost::lexical_cast<std::string>(player.maxhp);
	std::string shd_s = boost::lexical_cast<std::string>(player.shd);
	std::string shdm_s = boost::lexical_cast<std::string>(player.maxshd);
	sendPacket("0|A|HPT|" + hp_s + "|" + hpm_s);
	sendPacket("0|A|SHD|" + shd_s + "|" + shdm_s);
}

void CPlayerHandler::generateObjects(map_t mapid) {
	currentMap = Map(mapid);
	generatePortals();
	generateStations();
}
void CPlayerHandler::generatePortals() {
	for (auto& it : currentMap.getPortals()) {
		std::string createdPortal = sendpacket.createPortal(it.portalIDonMap, 1, 1, it.x, it.y);
		sendPacket(createdPortal);
	}
}
void CPlayerHandler::generateStations() {
	for (auto& it : currentMap.getStations()) {
		std::string createdStation = sendpacket.createStation(it.stationIDonMap, 1, it.stationName, it.company, 1,
			it.x, it.y);
		sendPacket(createdStation);
	}
}
void CPlayerHandler::generatePlayer() {
	for (auto& it : currentSession.getAllConnections()) {
		shipid_t shipID = boost::lexical_cast<shipid_t>(DBUtil::funcs::getShip(it->getID()));
		factionid_t companyID = boost::lexical_cast<factionid_t>(DBUtil::funcs::getCompany(it->getID()));
		rank_t rank = boost::lexical_cast<rank_t>(DBUtil::funcs::getRank(it->getID()));

		std::string spawnstring = sendpacket.spawnEnemy(it->getID(), shipID, 3, "PO", DBUtil::funcs::getUsername(it->getID()), it->getX() + 410, it->getY() + 300,
			companyID, 220, rank, false, 1, 4);

		sendPacket(spawnstring);
		sendPacket("0|n|d|" + boost::lexical_cast<std::string>(it->getID()) + "|" + DBUtil::funcs::getDrones(it->getID()));
	}
}
void CPlayerHandler::generatePlayer(id_t id) {
	handlePtrIt it = currentSession.getHandler(id);
	if (it != currentSession.getAllConnections().end()) {
		shipid_t shipID = DBUtil::funcs::getShip(getID());
		factionid_t companyID = DBUtil::funcs::getCompany(it->get()->getID());
		rank_t rank = DBUtil::funcs::getRank(it->get()->getID());

		std::string spawnstring = sendpacket.spawnEnemy(it->get()->getID(), shipID, 3, "PO", DBUtil::funcs::getUsername(it->get()->getID()), it->get()->getX() + 410, it->get()->getY() + 300,
			companyID, 220, rank, false, 1, 4);

		sendPacket(spawnstring);
		sendPacket("0|n|d|" + boost::lexical_cast<std::string>(it->get()->getID()) + "|" + DBUtil::funcs::getDrones(it->get()->getID()));
	}
}
void CPlayerHandler::checkForObjectsToInteract() {
	int portalID = 0;
	bool closeToGate = false;
	bool closeToStation = false;

	//for the sake of multiple read/writes these two for each loops have to create temporary new "Portals/Stations" instead of iterating through them.
	for (auto& it : currentMap.getPortals()) {
		if (isInRangeCircle(Portal::RANGE, it.x, it.y)) {
			portalID = it.portalIDonMap;
			closeToGate = true;
			break;
		}
	}
	for (auto& it : currentMap.getStations()) {
		if (isInRangeSquare(Station::RANGE, it.x - 510, it.y - 300)) {
			closeToStation = true;
			break;
		}
	}
	//TO-DO add other check, etc.

	std::string packet = sendpacket.events(closeToGate ? 1 : closeToStation ? 1 : 0, 0, closeToStation, 0, closeToGate, 0);
	sendPacket(packet);
}
bool CPlayerHandler::isInRangeCircle(int radius, pos_t objx, pos_t objy) {
	//must admit i couldnt really come up with this myself, guess this might be because its in the middle of the night right now, feel so ashamed :D
	Position_t position(mm->get_current_position());
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
	Position_t position(mm->get_current_position());
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
		disconnectUser();
		return;
	default:
		std::cout << "[Warning]\tUnknown error \"" << ec.value() << "\" at " << __FUNCTION__ << "(" << __LINE__ << "). Closing connection." << std::endl;
		std::cout << "[Warning]\tMessage: " << ec.message() << std::endl;
		disconnectUser();
		return;
	}

	if (bytes > 0 && ec.value() == 0) {
		using namespace Constants::Received; // incoming packet headers
		using namespace Constants; // for send, as it is not often used
		using boost::lexical_cast;
		std::istream is(&buffer);
		std::getline(is, information);
		fixPacket();
		//std::string output = "[" + boost::lexical_cast<std::string>(m_port) + " PACKET / " + boost::lexical_cast<std::string>(bytes) +  "|" + boost::lexical_cast<std::string>(information.length()) + "] " + information;
		//std::cout << output << std::endl;
		//write.writemore(output);

		static int dbgvar = 13000;
		if (packetStartsWith(W_POLICYFILEREQUEST)) {
			sendPacket(Send::W_CROSSDOMAINPOLICY);
		}
		else {
			std::vector<std::string> splitpackets;
			if (packetStartsWith(LOGIN_REQUEST)) {
				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));
				id = lexical_cast<id_t>(splitpackets.at(1));
				logInPackets();
			}
			else if (packetStartsWith("0")) {
				if (lexical_cast<rank_t>(player.rank) == 21)
					sendPacket(information);
			}
			else if (packetStartsWith(SETTINGS)) {
				//received: S|1|1|0|...
				//send: 0|S|1|1|0|..., we do not do anything with these informations yet
				sendPacket("0|" + information);
			}
			else if (packetStartsWith(MOVE)) {
				//determining the local and planned position
				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));

				/* LOGOUT BUTTON - SECURITY*/
				if (splitpackets.at(1) != "NaN" && splitpackets.at(2) != "NaN") {
					pos_t px = lexical_cast<pos_t>(splitpackets.at(1));
					pos_t py = lexical_cast<pos_t>(splitpackets.at(2));

					mm->move(px, py);
					Position_t position(mm->get_current_position());
					std::string move = sendpacket.move(id, position.first, position.second, px, py, player.speed,mm->get_time_for_destination());
					sendEveryone(move);
					std::cout << lexical_cast<std::string>(position.first) << "|" << lexical_cast<std::string>(position.second) << " - " << splitpackets.at(3) << "|" << splitpackets.at(4) << std::endl;
					checkForObjectsToInteract();
				}
				else
				{
					std::cout << "LOGOUT" << std::endl;

					//TODO: Save pos to DB
					pos_t posx = lexical_cast<pos_t>(splitpackets.at(3));
					pos_t posy = lexical_cast<pos_t>(splitpackets.at(4));
				}
			}
			else if (packetStartsWith(JUMP)) {
				if (!isJumping) {
					Position_t position = mm->get_current_position();
					static int oreval = 205000;
					sendPacket("0|r|" + lexical_cast<std::string>(oreval++) + "|1|" + lexical_cast<std::string>(position.first) + "|" + lexical_cast<std::string>(position.second));

					// make this a function u lazy ass
					std::shared_ptr<Portal> myPortal{ nullptr };
					for (auto& it : currentMap.getPortals()) {
						if (isInRangeCircle(500, it.x, it.y)) {
							isJumping = true;
							myPortal.reset(new Portal(it.portalIDonMap, it.x, it.y, it.destination_x, it.destination_y, it.destination_mapID));
							break;
						}
					}

					if (isJumping) {
						sendPacket("0|U|" + lexical_cast<std::string>(myPortal->destination_mapID) + "|" + lexical_cast<std::string>(myPortal->portalIDonMap));
						asyncThreads.push_back(async_func(2000,&CPlayerHandler::jump,myPortal->destination_mapID, myPortal->destination_x, myPortal->destination_y));
					}
				}
			}
			else if (packetStartsWith(LASERSHOOT_START)) {
				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));
				//thinking about doing adding this pointer as a 
				id_t uid = 0;
				uid = lexical_cast<id_t>(splitpackets.at(1));
				if (!isAttacking) {


					if (checkSelectedID(uid) && uid < 120000) { // >= 120.000 = NPC
						isAttacking = true;
					}
					//TODO: UID != LAST UID CHECK
					handleAttackRequest(uid);
				}
			}
			else if (packetStartsWith(ROCKETSHOOT)) {
				id_t uid = 0;

				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));

				uid = lexical_cast<id_t>(splitpackets.at(1));
				if (checkSelectedID(uid)) {
					damage_t damage = 0;
					std::string shoot;
					std::string bubbleMe;
					std::string bubbleEnemy;
					const handlePtrIt opponentHandle = currentSession.getHandler(uid);

					selectedOpponent = uid;
					if (!opponentHandle->get()->hasISH)
						damage = damageManager.damageRocket(uid, rockettype);
					shoot = sendpacket.rocketAttack(id, uid, rockettype, damage); //if damage == 0 hit is false
					makeDamage(damage, id, opponentHandle);

					if (opponentHandle != currentSession.getAllConnections().end()) {
						bubbleMe = sendpacket.damageBubbleSelected(opponentHandle->get()->getHP(), opponentHandle->get()->getSHD(), damage, false);
						bubbleEnemy = sendpacket.damageBubbleSelf(opponentHandle->get()->getHP(), opponentHandle->get()->getSHD(), damage);
					}
					else {
						//to at least keep the visuals if there is no one of that ID
						bubbleMe = sendpacket.damageBubbleSelected(1, 1, damage, false);
						bubbleEnemy = sendpacket.damageBubbleSelf(1, 1, damage);
					}

					sendEveryone(shoot);
					sendPacket(bubbleMe);
					currentSession.sendTo(bubbleEnemy, selectedOpponent);
					sendPacketAfter(2000, "0|A|RCD");
					// for some reason I had huge problems with this small task thread, after the 50th rocket it had a chance to crash. it came out of no apparent reason but since
					// i included boost\thread.hpp it seemed to work and I don't really know whats the reason behind it
				}
				else {
					sendPacket("0|A|RCD");
				}
			}
			else if (packetStartsWith(LASERAMMO_CHANGE)) {
				lasertype_t laserid = 0;

				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));

				laserid = lexical_cast<lasertype_t>(splitpackets.at(1));
				//if the player hasn't the big laser in the front make the id 1 to id 0 (red)
				//REMOVE THIS AS FAST AS POSSIBLE LaserType != lasertype_t						
				//LaserType = color			lasertype_t = ammo
				lasertype = static_cast<LaserType>(laserid);
				if (isAttacking) {
					std::string attackLaser = sendpacket.laserAttack(id, selectedOpponent, lasertype);
					sendEveryone(attackLaser);
				}
			}
			else if (packetStartsWith(ROCKETAMMO_CHANGE)) {
				rockettype_t rocketid = 0;

				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));
				rocketid = lexical_cast<rockettype_t>(splitpackets.at(1));
				rockettype = static_cast<RocketType>(rocketid);
			}
			else if (packetStartsWith(LASERSHOOT_ABORT)) {
				//id_t uid = 0;
				//std::string abort;
				//boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));

				//uid = lexical_cast<id_t>(splitpackets.at(1));
				//abort = sendpacket.laserAbort(id, uid);
				isAttacking = false;

				//sendEveryone(abort);
			}
			else if (packetStartsWith(SPECIAL_SMARTBOMB)) {
				std::string userID_string = lexical_cast<std::string>(id);
				sendEveryone("0|n|SMB|" + userID_string);
				sendPacket("0|A|CLD|SMB|10");
				isInSMBCooldown = true;


				asyncThreads.push_back(async_func(10000,&CPlayerHandler::detonateSMB));


				for (auto& it : currentSession.getAllConnections()) {
					if (isInRangeCircle(500, it->getX(), it->getY()) && !(it->hasISH) && it->getID() != id && it->getID() < 120000) {
						it->receiveDamageHP(it->getHP() * 0.2, id);


						///sendEveryone("0|n|ISH|" + lexical_cast<std::string>(it->getID())); // prove of who got hit by the smb
					}
				}
			}
			else if (packetStartsWith(SPECIAL_INSTASHIELD)) {
				std::string userID_string = lexical_cast<std::string>(id);
				sendEveryone("0|n|ISH|" + userID_string);
				sendPacket("0|A|CLD|ISH|10");
				hasISH = true;

				asyncThreads.push_back(async_func(3000,&CPlayerHandler::detonateISH));

			}
			else if (packetStartsWith(SELECT)) {
				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));
				selectedOpponent = lexical_cast<id_t>(splitpackets.at(1));
				id_t uid = selectedOpponent;
				if (checkSelectedID(uid)) {
					handlePtrIt opponentHandle;
					std::string selectShowBars;
					health_t normal_hp_opp = 100;
					health_t max_hp_opp = 100;
					shield_t normal_shd_opp = 100;
					shield_t max_shd_opp = 100;

					opponentHandle = currentSession.getHandler(uid);
					if (opponentHandle != currentSession.getAllConnections().end()) {
						normal_hp_opp = opponentHandle->get()->getHP();
						max_hp_opp = opponentHandle->get()->getMaxHP();
						normal_shd_opp = opponentHandle->get()->getSHD();
						max_shd_opp = opponentHandle->get()->getMaxSHD();
					}
					selectShowBars = sendpacket.showBars(normal_hp_opp, max_hp_opp, normal_shd_opp, max_shd_opp);
					sendPacket(selectShowBars);
				}
			}
			else if (packetStartsWith(FORCEINIT)) {
				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));
				id_t init_id = lexical_cast<id_t>(splitpackets.at(1));
				generatePlayer(init_id);
			}
#ifdef DBG_PACKETCLI
			else if (packetStartsWith("DBG_PACKETTEST")) {
				sendPacket("0|A|STD|testmsg1");
			}
			else if (packetStartsWith("DBG_SPAWN1")) {
				std::string spawnmobstring = sendpacket.spawnEnemy(dbgvar++, 53, 3, "TEST", "TEST NAME 1", getX() + 410, getY() + 300, 0, 0, 0, false, 0, 0, false);
				std::cout << spawnmobstring << std::endl;
				sendPacket(spawnmobstring);
			}
			else if (packetStartsWith("DBG_SPAWN2")) {
				std::string spawnmobstring = sendpacket.spawnEnemy(dbgvar++, 54, 3, "TEST", "TEST NAME 2", getX() + 410, getY() + 300, 0, 0, 0, false, 0, 0, false);
				std::cout << spawnmobstring << std::endl;
				sendPacket(spawnmobstring);
			}
			else if (packetStartsWith("DBG_SPAWN3")) {
				std::string spawnmobstring = sendpacket.spawnEnemy(dbgvar++, 73, 0, "TEST", "TEST NAME 3", getX() + 410, getY() + 300, 0, 0, 0, false, 0, 0, false);
				std::cout << spawnmobstring << std::endl;
				sendPacket(spawnmobstring);
			}
			else if (packetStartsWith("STAT")) {
				static stationid_t stationID = 1;
				pos_t statx = 0;
				pos_t staty = 0;
				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));
				try {
					/* Station Pos */
					/*x*/
					if (splitpackets.at(1) == "H" || splitpackets.at(1) == "h") {
						statx += getX() + 1;
					}
					else {
						statx += lexical_cast<pos_t>(splitpackets.at(1));
					}
					/*y*/
					if (splitpackets.at(2) == "H" || splitpackets.at(2) == "h") {
						staty += getY() + 1;
					}
					else {
						staty += lexical_cast<pos_t>(splitpackets.at(2));
					}
				}
				catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
				catch (const boost::exception& e) { std::cerr << BOOST_CERR_OUTPUT(e) << std::endl; }

				std::string stationstring = sendpacket.createStation(stationID++, 1, "basicStation", 3, 1, statx, staty);
				sendPacket(stationstring);
			}
			else if (packetStartsWith("MAP")) {
				map_t wantedMapID = 1;
				pos_t dest_x = 0;
				pos_t dest_y = 0;

				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));
				try {
					wantedMapID = lexical_cast<map_t>(splitpackets.at(1));
					dest_x = lexical_cast<pos_t>(splitpackets.at(2));
					dest_y = lexical_cast<pos_t>(splitpackets.at(3));
				}
				catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
				catch (const boost::exception& e) { std::cerr << BOOST_CERR_OUTPUT(e) << std::endl; }
				jump(wantedMapID, dest_x, dest_y);
			}
			else if (packetStartsWith("SPAWNIT")) {
				static id_t mobID = 120000;
				shipid_t mob_shipID = 71;
				std::string mob_name = "<undefined>";
				pos_t mob_x = 410; //g_OffsetX = 410 -> for information see /frame 10/DoAction_30 in spacemap.swf
				pos_t mob_y = 300; //g_OffsetY = 300 -> for information see /frame 10/DoAction_30 in spacemap.swf

				boost::algorithm::split(splitpackets, information, boost::is_any_of("|"));
				try {
					/* Mob ShipType*/
					mob_shipID = lexical_cast<shipid_t>(splitpackets.at(1));
					if (mob_shipID < 71 || mob_shipID > 90) {
						mob_shipID = 71; 
					}

					/* Mob Name*/
					mob_name = "-=[ " + splitpackets.at(2) + " ]=-";


					/* Mob Pos */
					/*x*/
					if (splitpackets.at(3) == "H" || splitpackets.at(3) == "h") {
						mob_x += getX() + 1;
					}
					else {
						mob_x += lexical_cast<pos_t>(splitpackets.at(3));
					}
					/*y*/
					if (splitpackets.at(4) == "H" || splitpackets.at(4) == "h") {
						mob_y += getY() + 1;
					}
					else {
						mob_y += lexical_cast<pos_t>(splitpackets.at(4));
					}
				}
				catch (const std::exception& e) { std::cerr << STD_CERR_OUTPUT(e) << std::endl; }
				catch (const boost::exception& e) { std::cerr << BOOST_CERR_OUTPUT(e) << std::endl; }


				std::string spawnmobstring = sendpacket.spawnEnemy(mobID++, mob_shipID, 0, "", mob_name, mob_x, mob_y, 0, 0, 0, false, 0, 0, false);
				std::cout << spawnmobstring << std::endl;
				sendPacket(spawnmobstring);
			}
#endif
		} // END OF !IF POLICY REQUEST
	}//END OF IF BYTES > 0
	readData();
}

void CPlayerHandler::sendEveryone(std::string packet) {
	allInSession[currentMap.getMapId()].sendEveryone(packet, shared_from_this());
}

void CPlayerHandler::receiveDamagePure(damage_t dmg, id_t from)
{
	constexpr const double SHIELDPERCENTAGE = 0.7;
	/* Damage split between shield and HP */
	damage_t shield_damage = dmg * SHIELDPERCENTAGE;
	damage_t health_damage = dmg - shield_damage; //fight me, if you want to have a (1 - SHIELDPERCENTAGE) 

	/* HP / SHD after hit */
	health_t health_post_damage = player.hp - health_damage;
	shield_t shield_post_damage = player.shd - shield_damage;

	if (shield_post_damage < 0) {
		health_post_damage += shield_post_damage; //s_p_d is negative so we add that to our health 
		player.shd = 0;
	}
	else {
		player.shd = shield_post_damage;
	}

	if (health_post_damage <= 0) {
		player.hp = 0;
		die();
	}
	else {
		player.hp = health_post_damage;
	}

	updateHealth();
}

void CPlayerHandler::receiveDamageHP(damage_t dmg, id_t from) {
	player.hp -= dmg;

	// to ensure no overflow happens
	if (player.hp < 0) {
		die();
		player.hp = 0;
	}

	updateHealth();
}

void CPlayerHandler::receiveDamageSHD(damage_t dmg, id_t from) {
	player.shd -= dmg;

	// to ensure no overflow happens
	if (player.shd < 0) {
		player.shd = 0;
	}

	updateHealth();
}

void CPlayerHandler::die()
{
	try {
		sendEveryone(sendpacket.kill(id));
		currentSession.leaveSession(shared_from_this());
		disconnectUser();
	}
	catch (boost::exception_ptr e) {
		std::cerr << BOOST_CERR_OUTPUT(e) << std::endl;
	}
	catch (std::exception e) {
		std::cerr << STD_CERR_OUTPUT(e) << std::endl;
	}
}

void CPlayerHandler::fixPacket() {
	try {
		//We are going to erase all annoying little bytes of chars that are hindering us, only which might! happen to prevent the server from taking too long with this
		bool triggered = false;
		for (auto character : information) {
			if (character == '\n') triggered = true;
		}
		if (triggered) {
			write.writemore("[WARNING] NEW LINE FEED IN PACKET STRING: " + information);
		}
		///boost::erase_all(information, "\n"); // new line feed (\n)
		if (information.at(0) == 0x0 && information.size() > 1) { //in most cases a null terminator is at 0 due to the socket sending packets condition so yeah lets remove it 
			information = information.substr(1);
		}
	}
	catch (std::exception& ex) {
		std::cerr << STD_CERR_OUTPUT(ex) << std::endl;
	}
	catch (boost::exception& ex) {
		std::cerr << BOOST_CERR_OUTPUT(ex) << std::endl;
	}
}

template<class callable, class ...arguments>
std::thread* CPlayerHandler::async_func(callable && func, arguments&&... args)
{
	return new std::thread(func, shared_from_this(), args...);
}
template<class callable, class ...arguments>
std::thread* CPlayerHandler::async_func(int ms,callable && func, arguments&&... args)
{
	auto f = std::bind(func, shared_from_this(), args...);
	std::thread* thread_handle = new std::thread([f,ms]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		f();
	});
	return thread_handle;
}
