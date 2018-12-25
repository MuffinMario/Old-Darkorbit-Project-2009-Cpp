
#include "Session.h"
#include "Mob.h"
#include "Collectable.h"
// should i #include "BonusBox.h"?
std::map<id_t, std::mutex> g_emutex;
std::map<id_t, std::mutex> g_collmutex;
CSession::ConnectionContainer_t& CSession::getAllConnections() { return m_connections; }

void CSession::joinSession(handlePtr connection,std::string sessionId)
{
	/* mutexes are in functions */
	if (containsSessionId(sessionId))
	{
		warncout << "[" << m_map.getMapId() << "] [INFO] SessionId " << sessionId << " already online! (replacing...)" << cendl;
		leaveSession(sessionId);
	}

	lockConnectionsWrite();
	m_connections.insert(make_pair(sessionId, connection));
	unlockConnectionsWrite();

	dcout << "["  << m_map.getMapId() << "] joined, size: " << m_connections.size() << cendl;
}
void CSession::leaveSession(std::string sessionId)
{
	dcout << "[" << m_map.getMapId() << "] before left, size: " << m_connections.size() << cendl;
	try {
		CPacketComposer sp;
		lockConnectionsRead();
		id_t id = m_connections.at(sessionId)->getID();
		m_connections.at(sessionId)->suspendThreads();
		sendEveryone(sp.removeOpponent(id));
	}
	catch (std::out_of_range& oor)
	{
		warncout << "[" << m_map.getMapId() << "]SessionID " << sessionId << " out of range while leaving session" << cendl;
	}
	unlockConnectionsRead();
	lockConnectionsWrite();
	m_connections.erase(sessionId);
	unlockConnectionsWrite();
	dcout << "[" << m_map.getMapId() << "] left, size: " << m_connections.size() << cendl;
}

bool CSession::containsSessionId(const std::string& sessionId)
{
	bool found = false;
	lockConnectionsRead();
	if (m_connections.size() > 0)
	{
		found = std::any_of(m_connections.begin(), m_connections.end(), [&sessionId](const std::pair<std::string, handlePtr>& connection) {
			return connection.first == sessionId;
		});
	}
	unlockConnectionsRead();
	return found;
}

void CSession::addMob(std::shared_ptr<CMob> m)
{
	if (m->getId() > m_highestMobId) m_highestMobId = m->getId();

	lockMobsWrite();
	m_NPCs.push_back(m);
	unlockMobsWrite();
}
void CSession::addCollectable(std::shared_ptr<ICollectable> c)
{
	if (c->getId() > m_highestCollectableId) m_highestCollectableId = c->getId();
	
	lockCollectablesWrite();
	m_collectables.push_back(c);
	unlockCollectablesWrite();
}
CSession::NpcContainer_t::iterator CSession::removeMob(id_t id)
{
	lockMobsWrite();
	for (auto it =m_NPCs.begin(); it != m_NPCs.end();)
	{
		if (it._Ptr) //no i will rethink this later
		{

			std::shared_ptr<CMob> mob = *it;
			
			if (mob)
			{
				if (mob->getId() == id)
				{
					mob.reset();
					it = m_NPCs.erase(it);
					unlockMobsWrite();
					return it;
				}
				else
				{
					it++;
				}
			}
		}
	}
	unlockMobsWrite();
	return m_NPCs.end();
}

CSession::Collectables_t::iterator CSession::removeCollectable(id_t id)
{
	lockCollectablesWrite();
	for (auto it = m_collectables.begin(); it != m_collectables.end();)
	{
		if (it._Ptr)
		{

			std::shared_ptr<ICollectable> coll = *it;

			if (coll)
			{
				if (coll->getId() == id)
				{
					coll.reset();
					it = m_collectables.erase(it);
					unlockCollectablesWrite();
					return it;
				}
				else
				{
					it++;
				}
			}
		}
	}
	unlockCollectablesWrite();
	return m_collectables.end();
}

id_t CSession::generateNewCollectableId()
{
	return ++m_highestCollectableId;
}
id_t CSession::generateNewMobId()
{
	return ++m_highestMobId;
}

map_t CSession::getMapId()
{
	return m_map.getMapId();
}

CMap CSession::getMap()
{
	return m_map;
}


std::shared_ptr<CMob> CSession::getMob(id_t id)
{
	lockMobsRead();
	for (size_t i = 0; i < m_NPCs.size(); i++)
	{
		if (m_NPCs.at(i)->getId() == id)
		{
			auto ptr = m_NPCs.at(i);
			unlockMobsRead();
			return ptr;
		}
	}
	unlockMobsRead();
	return nullptr;
	//throw std::out_of_range("\"id\" not inside m_NPCs!");
}
std::shared_ptr<ICollectable> CSession::getCollectable(id_t id)
{
	lockCollectablesRead();
	for (size_t i = 0; i < m_collectables.size(); i++)
	{
		if (m_collectables.at(i)->getId() == id)
		{
			auto ptr = m_collectables.at(i);
			unlockCollectablesRead();
			return ptr;
		}
	}
	unlockCollectablesRead();
	return nullptr;
}
CSession::NpcContainer_t & CSession::getMobs()
{
	return m_NPCs;
}
CSession::Collectables_t & CSession::getCollectables()
{
	return m_collectables;
}
#define DEBUG_PRINT_CONNECTION_MUTEX 0
#define DEBUG_PRINT_MOBS_MUTEX 0
#define DEBUG_PRINT_COLLECTABLES_MUTEX 0

/* locks */
void CSession::lockConnectionsWrite()
{
#if DEBUG_PRINT_CONNECTION_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif
	m_connectionMutex->lock();
}

void CSession::lockMobsWrite()
{

#if DEBUG_PRINT_MOBS_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif	
	m_npcMutex->lock();
}

void CSession::lockCollectablesWrite()
{
#if DEBUG_PRINT_COLLECTABLES_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif	

	m_collectableMutex->lock();
}
void CSession::lockConnectionsRead()
{

#if DEBUG_PRINT_CONNECTION_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif	
	m_connectionMutex->lock_shared();
}

void CSession::lockMobsRead()
{

#if DEBUG_PRINT_MOBS_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif	
	m_npcMutex->lock_shared();
}

void CSession::lockCollectablesRead()
{

#if DEBUG_PRINT_COLLECTABLES_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif	
	m_collectableMutex->lock_shared();
}
/* unlocks */
void CSession::unlockConnectionsWrite()
{

#if DEBUG_PRINT_CONNECTION_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif
	m_connectionMutex->unlock();
}

void CSession::unlockMobsWrite()
{
#if DEBUG_PRINT_MOBS_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif	
	m_npcMutex->unlock();
}

void CSession::unlockCollectablesWrite()
{

#if DEBUG_PRINT_COLLECTABLES_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif		
	m_collectableMutex->unlock();
}

void CSession::unlockConnectionsRead()
{

#if DEBUG_PRINT_CONNECTION_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif	
	m_connectionMutex->unlock_shared();
}

void CSession::unlockMobsRead()
{
#if DEBUG_PRINT_MOBS_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif	
	m_npcMutex->unlock_shared();
}

void CSession::unlockCollectablesRead()
{
#if DEBUG_PRINT_COLLECTABLES_MUTEX == 1
	dcout << __FUNCTION__ << cendl;
#endif	
	m_collectableMutex->unlock_shared();
}

const size_t CSession::getMobCount()
{
	return m_NPCs.size();
}

const size_t CSession::getCollectableCount()
{
	return m_collectables.size();
}


//unused
void CSession::clearOldSession(handlePtr connection)
{
	id_t userID = connection->getID();
	lockConnectionsWrite();
	for(auto it = m_connections.begin();it != m_connections.end();){
		if (userID == it->second->getID()) {
			it = m_connections.erase(it);
		}
		else {
			++it;
		}
	}
	unlockConnectionsWrite();
}
void CSession::sendEveryone(std::string str)
{
	if (m_connections.size() < 1) return;

	lockConnectionsRead();
	for (auto& it : m_connections) {
		it.second->sendPacket(str);
	}
	unlockConnectionsRead();
}

void CSession::sendEveryoneBut(std::string str, const std::string& sessionID)
{
	lockConnectionsRead();
	if (m_connections.size() < 1) return;
	for (auto& it : m_connections) {
		if (it.first != sessionID)
			it.second->sendPacket(str);
	}
	unlockConnectionsRead();
}

void CSession::sendEveryoneBut(std::string str, id_t userID)
{
	lockConnectionsRead();
	if (m_connections.size() < 1) return;
	for (auto& it : m_connections) {
		if(it.second->getID() != userID)
			it.second->sendPacket(str);
	}
	unlockConnectionsRead();
}

void CSession::sendEveryoneBut(std::string str, std::function<bool(handlePtr)>& det)
{
	lockConnectionsRead();
	if (m_connections.size() < 1) return;
	for (auto& it : m_connections) {
		if (it.second && det(it.second))
			it.second->sendPacket(str);
	}
	unlockConnectionsRead();
}

void CSession::sendTo(std::string str, std::string sessionId) {
	//in case of tight race condition
	if(containsSessionId(sessionId))
		m_connections[sessionId]->sendPacket(str);
}

void CSession::sendTo(std::string str, id_t userID)
{
	lockConnectionsRead();

	handlePtrIt connection = std::find_if(m_connections.begin(), m_connections.end(), [&userID](const std::pair<std::string, handlePtr>& con) {
		return con.second->getID() == userID;
	});

	if (connection != m_connections.end())
	{
		connection->second->sendPacket(str);
	}
	unlockConnectionsRead();
}

//returns the shared_ptr to the first result of this id
const handlePtr CSession::getHandler(id_t id) {
	handlePtr handle = nullptr;
	lockConnectionsRead();
	handlePtrIt connection = std::find_if(m_connections.begin(), m_connections.end(), [&id](const std::pair<std::string, handlePtr>& con) {
		return con.second->getID() == id;
	});
	if (connection != m_connections.end())
	{
		handle = connection->second;
	}
	/*on other threads: ref counts of this handle could be 0, but due to ^ its always >0 since we 
	 * prohibited the elimination of the container element with this
	 */
	unlockConnectionsRead();
	return handle;
}