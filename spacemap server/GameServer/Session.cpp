
#include "Session.h"
#include "Mob.h"
#include "Collectable.h"
// should i #include "BonusBox.h"?
std::map<id_t, std::mutex> g_emutex;
std::map<id_t, std::mutex> g_collmutex;
extern std::map<map_t, CMap> g_allMaps;
CSession::ConnectionContainer_t& CSession::getAllConnections() { return m_connections; }

void CSession::joinSession(handlePtr connection,id_t userID)
{
	/* mutexes are in functions */
	if (containsUserId(userID))
	{
		warncout << "[" << m_map.getMapId() << "] [INFO] userID " << userID << " already online! (replacing...)" << cendl;
		leaveSession(userID);
	}

	lockConnectionsWrite();
	m_connections.insert(make_pair(userID, connection));
	unlockConnectionsWrite();

	dcout << "["  << m_map.getMapId() << "] joined, size: " << m_connections.size() << cendl;
}
void CSession::leaveSession(id_t userID)
{
	dcout << "[" << m_map.getMapId() << "] before left, size: " << m_connections.size() << cendl;
	lockConnectionsRead();
	try {
		CPacketComposer sp;
		sendEveryone(sp.removeOpponent(userID));
	}
	catch (std::out_of_range& oor)
	{
		warncout << "[" << m_map.getMapId() << "]userID " << userID << " out of range while leaving session" << cendl;
	}
	unlockConnectionsRead();
	lockConnectionsWrite();
	m_connections.erase(userID);
	unlockConnectionsWrite();
	dcout << "[" << m_map.getMapId() << "] left, size: " << m_connections.size() << cendl;
}

bool CSession::containsUserId(id_t userID)
{
	bool found = false;
	lockConnectionsRead();
	found = m_connections.find(userID) != m_connections.end();
	unlockConnectionsRead();
	return found;
}

CSession::CSession(map_t mapid) :m_map(g_allMaps[mapid]),
m_connectionMutex(new std::shared_mutex),
m_npcMutex(new std::shared_mutex),
m_collectableMutex(new std::shared_mutex)
{
	//dcout << "Creating session with mapid " << mapid << cendl;
}

void CSession::addMob(std::shared_ptr<CMob> m)
{
	if (m->getId() > m_highestMobId) m_highestMobId = m->getId();

	lockMobsWrite();
	m_NPCs[m->getId()] = m;
	unlockMobsWrite();
}
void CSession::addCollectable(std::shared_ptr<ICollectable> c)
{
	if (c->getId() > m_highestCollectableId) m_highestCollectableId = c->getId();
	
	lockCollectablesWrite();
	m_collectables[c->getId()] = c;
	unlockCollectablesWrite();
	c->spawn();
}
CSession::NpcContainer_t::iterator CSession::removeMob(id_t id)
{
	lockMobsWrite();
	m_NPCs.erase(id);
	/*for (auto it =m_NPCs.begin(); it != m_NPCs.end();)
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
	}*/
	unlockMobsWrite();
	return m_NPCs.end(); //TODO
}

CSession::Collectables_t::iterator CSession::removeCollectable(id_t id)
{
	lockCollectablesWrite();
	m_collectables.erase(id);
	/* old and old for (auto it = m_collectables.begin(); it != m_collectables.end();)
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
	}*/
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

CMap& CSession::getMap()
{
	return m_map;
}


std::shared_ptr<CMob> CSession::getMob(id_t id)
{
	lockMobsRead();
	NpcContainer_t::iterator found = m_NPCs.find(id);
	if (found != m_NPCs.end())
	{
		unlockMobsRead();
		return (*found).second;
	}
	unlockMobsRead();
	return nullptr;
	//throw std::out_of_range("\"id\" not inside m_NPCs!");
}
std::shared_ptr<ICollectable> CSession::getCollectable(id_t id)
{
	lockCollectablesRead();
	Collectables_t::iterator found = m_collectables.find(id);
	if (found != m_collectables.end())
	{
		unlockCollectablesRead();
		return (*found).second;
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

void CSession::foreachCollectable(std::function<void(id_t,std::shared_ptr<ICollectable>&)> f)
{
	std::shared_lock<std::shared_mutex> lock(*m_collectableMutex);
	for (auto& c : m_collectables)
	{
		f(c.first,c.second);
	}
}

void CSession::foreachMobs(std::function<void(id_t, std::shared_ptr<CMob>&)> f)
{
	std::shared_lock<std::shared_mutex> lock(*m_npcMutex);
	for (auto& m : m_NPCs)
	{
		f(m.first, m.second);
	}
}

void CSession::foreachConnections(std::function<void(id_t, handlePtr&)> f)
{
	std::shared_lock<std::shared_mutex> lock(*m_connectionMutex);
	for (auto& c : m_connections)
	{
		f(c.first, c.second);
	}
}

void CSession::foreachCollectable(std::function<void(id_t, std::shared_ptr<ICollectable>&)> f, std::function<bool(id_t, std::shared_ptr<ICollectable>&)> pred)
{
	std::shared_lock<std::shared_mutex> lock(*m_collectableMutex);
	for (auto& c : m_collectables)
	{
		if (pred(c.first, c.second))
			f(c.first, c.second);
	}
}

void CSession::foreachMobs(std::function<void(id_t, std::shared_ptr<CMob>&)> f, std::function<bool(id_t, std::shared_ptr<CMob>&)> pred)
{
	std::shared_lock<std::shared_mutex> lock(*m_npcMutex);
	for (auto& m : m_NPCs)
	{
		if(pred(m.first,m.second))
			f(m.first, m.second);
	}
}

void CSession::foreachConnections(std::function<void(id_t, handlePtr&)> f, std::function<bool(id_t, handlePtr&)> pred)
{
	std::shared_lock<std::shared_mutex> lock(*m_connectionMutex);
	for (auto& c : m_connections)
	{
		if (pred(c.first, c.second))
			f(c.first, c.second);
	}
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

void CSession::sendEveryoneBut(std::string str, id_t userID)
{
	lockConnectionsRead();
	if (m_connections.size() < 1)
	{	
		unlockConnectionsRead();
		return;
	}
	for (auto& it : m_connections) {
		if(it.first != userID)
			it.second->sendPacket(str);
	}
	unlockConnectionsRead();
}

void CSession::sendEveryoneBut(std::string str, std::function<bool(handlePtr)>& det)
{
	lockConnectionsRead();
	if (m_connections.size() < 1)
	{
		unlockConnectionsRead();
		return;
	}
	for (auto& it : m_connections) {
		if (it.second && det(it.second))
			it.second->sendPacket(str);
	}
	unlockConnectionsRead();
}

void CSession::sendTo(std::string str, id_t userID)
{
	lockConnectionsRead();

	handlePtr connection = m_connections[userID];

	if (connection != nullptr)
	{
		connection->sendPacket(str);
	}
	unlockConnectionsRead();
}

//returns the shared_ptr to the first result of this id
const handlePtr CSession::getHandler(id_t id) {
	lockConnectionsRead();
	ConnectionContainer_t::iterator found = m_connections.find(id);
	if (found != m_connections.end())
	{
		unlockConnectionsRead();
		return (*found).second;
	}
	/*on other threads: ref counts of this handle could be 0, but due to ^ its always >0 since we 
	 * prohibited the elimination of the container element with this
	 */
	unlockConnectionsRead();
	return nullptr;
}