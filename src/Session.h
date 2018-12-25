#ifndef SINGLE_SESSION_H
#define SINGLE_SESSION_H
#include <boost\shared_ptr.hpp>
#include <boost\bind.hpp>
#include <map>
#include <vector>
#include <functional>
#include <Windows.h>
#include <shared_mutex>
#include "Handler.h"
#include "Map.h"

class CMob;
class ICollectable;
class CBonusBox;

class CSession
{
public:
	/* Typedefs */

	typedef std::map<std::string, handlePtr> ConnectionContainer_t;
	typedef std::vector<std::shared_ptr<CMob>> NpcContainer_t;
	typedef std::vector<std::shared_ptr<ICollectable>> Collectables_t;
	typedef std::unique_ptr<std::shared_mutex> mutex_t;
private:
	mutex_t m_connectionMutex;
	mutex_t m_npcMutex;
	mutex_t m_collectableMutex;

	CMap m_map;
	ConnectionContainer_t m_connections;
	NpcContainer_t m_NPCs;
	Collectables_t m_collectables;

	id_t m_highestMobId = 0LL;
	id_t m_highestCollectableId = 0LL;
	//returns true if a client (session id) is already connected to the server
	bool				containsSessionId(const std::string& sessionId);
public:
	explicit CSession(map_t mapid) :m_map(mapid),
		m_connectionMutex(new std::shared_mutex),
		m_npcMutex(new std::shared_mutex),
		m_collectableMutex(new std::shared_mutex)
	{
		//dcout << "Creating session with mapid " << mapid << cendl;
	}
	
	/* Getters */

	/* Returns reference to STL container */
	ConnectionContainer_t&		getAllConnections();
	NpcContainer_t&				getMobs();
	Collectables_t&				getCollectables();

	/* Mutexes */
	void lockConnectionsWrite();
	void lockMobsWrite();
	void lockCollectablesWrite();
	void lockConnectionsRead();
	void lockMobsRead();
	void lockCollectablesRead();

	void unlockConnectionsWrite();
	void unlockMobsWrite();
	void unlockCollectablesWrite();
	void unlockConnectionsRead();
	void unlockMobsRead();
	void unlockCollectablesRead();

	/* Returns new shared ptr reference*/
	const handlePtr					getHandler(id_t id);
	std::shared_ptr<CMob>			getMob(id_t id);
	std::shared_ptr<ICollectable>	getCollectable(id_t id);

	const size_t getMobCount();
	const size_t getCollectableCount();

	/* Session Connection Functions */

	//makes a client join the session
	void				joinSession(const handlePtr connection,std::string sessionId);
	//makes a client leave the session
	void				leaveSession(std::string sessionId);


	/* Session Connection Socket Functions */
	void				sendEveryone(std::string str) ;
	void				sendEveryoneBut(std::string str, const std::string& sessionID);
	void				sendEveryoneBut(std::string str, id_t userID);
	/* Relatively more complex function which includes a function that lets the user implement a delimiter 
		example sendEveryoneBut("0|A|STD|Hello User",[](handlePtr ptr) { return ptr->getID() < 20;)
			sends everyone whose userID is below 20 the message "Hello User"
			! has nullpointer check internally
	*/
	void				sendEveryoneBut(std::string str, std::function<bool(handlePtr)>& det);

	//sends a string-packet to a userID
	void				sendTo(std::string str, std::string sessionId);
	void				sendTo(std::string str, id_t userID);


	void addMob(std::shared_ptr<CMob> m);
	void addCollectable(std::shared_ptr<ICollectable> c);
	/* Returns iterator if in nested iteration */
	NpcContainer_t::iterator removeMob(id_t id);
	/* Returns iterator if in nested iteration */
	Collectables_t::iterator removeCollectable(id_t id);
	/* This function increments the highest collectable ID and should be used simultaneously with addCollectable */
	id_t generateNewCollectableId();
	/* This function increments the highest mob ID and should be used simultaneously with addMob */
	id_t generateNewMobId();
	map_t getMapId();
	CMap getMap();
private:

	//checks if there are already IDs connected and if they do they get erased so only one Handler with only one ID is there
	void clearOldSession(handlePtr connection);
};
#endif