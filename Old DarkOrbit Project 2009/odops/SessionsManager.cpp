
#include "SessionsManager.h"
#include "Mob.h"
std::map<id_t, std::mutex> g_emutex;
CSessionsManager::ConnectionContainer_t CSessionsManager::getAllConnections() { return m_connections; }

void CSessionsManager::joinSession(handlePtr connection,std::string sessionId)
{
	for (auto i = m_connections.begin(); i != m_connections.end(); i++)
	{
		dcout << "SessionId: " << i->first << " Counts: " << i->second.use_count() << cendl;
	}
	if (containsSessionId(sessionId))
	{
		cout << EColor::YELLOW << "[INFO] SessionId " << sessionId << " already online! (replacing...)" << cendl;
		leaveSession(sessionId);
	}
	m_connections.insert(make_pair(sessionId, connection));
	dcout << "joined, size: " << m_connections.size() << cendl;
}
void CSessionsManager::leaveSession(std::string sessionId)
{
	dcout << "before left, size: " << m_connections.size() << cendl;
	id_t id = m_connections.at(sessionId)->getID();
	g_emutex[id].lock();
	m_connections.erase(sessionId);
	g_emutex[id].unlock();
	dcout << "(left, size: " << m_connections.size() << cendl;
}

bool CSessionsManager::containsSessionId(std::string sessionId)
{
	if (m_connections.size() > 0)
	{
		for (auto& it : m_connections)
		{
			if (it.first == sessionId)
			{
				return true;
			}
		}
	}
	return false;
}

void CSessionsManager::addMob(std::shared_ptr<CMob> m)
{
	if (m->getId() > m_highestId)
		m_highestId = m->getId();
	//TODO if(idOfMobAlreadyExists(m))
	m_NPCs.push_back(m);
}
void CSessionsManager::removeMob(id_t id)
{
	for (auto it =m_NPCs.begin(); it != m_NPCs.end();it++)
	{
		if ((*it)->getId() == id)
		{
			dcout << "Erasing " << id << "!" << cendl;
			m_NPCs.erase(it);
		}
	}
}

std::shared_ptr<CMob> CSessionsManager::getMob(id_t id)
{
	for (int i = 0; i < m_NPCs.size(); i++)
	{
		if (m_NPCs.at(i)->getId() == id)
		{
			return m_NPCs.at(i);
		}
	}
	return nullptr;
	//throw std::out_of_range("\"id\" not inside m_NPCs!");
}
const id_t CSessionsManager::getMobHighestId()
{
	return m_highestId;
}

const size_t CSessionsManager::getMobCount()
{
	return m_NPCs.size();
}

//unused
void CSessionsManager::clearOldSession(handlePtr connection)
{
	int userID = connection->getID();
	for(auto it = m_connections.begin();it != m_connections.end();it++){
		if (userID == it->second->getID()) {
			m_connections.erase(it);
		}
	}
}
void CSessionsManager::sendEveryone(std::string str) const
{
	if (m_connections.size() < 1) return;
	for (auto& it : m_connections) {
		it.second->sendPacket(str);
	}
}

void CSessionsManager::sendTo(std::string str, std::string sessionId) {
	//in case of tight race condition
	if(containsSessionId(sessionId))
		m_connections[sessionId]->sendPacket(str);
}

//returns the iterator to the first result of this id, maybe something else instead of an iterator since this is not what an iterator is ment to be used to
// WARNING: PLEASE CONSIDER ALWAYS DOING A IT != container.end() CHECK
const handlePtrIt CSessionsManager::getHandler(id_t id) {
	for (handlePtrIt it = m_connections.begin(); it != m_connections.end(); it++)
	{
		if (it->second->getID() == id)
		{
			return it;
		}
	}
	throw std::out_of_range("Handler not found");
}