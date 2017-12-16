#ifndef SESSIONSMANAGER_H
#define SESSIONSMANAGER_H
#include <boost\shared_ptr.hpp>
#include <boost\bind.hpp>
#include <map>
#include <vector>
#include <Windows.h>
#include "Handler.h"
#include "map.h"
class CMob;

class CSessionsManager
{
public:
	//std::map because sessionID
	typedef std::map<std::string, handlePtr> ConnectionContainer_t;
	typedef std::vector<std::shared_ptr<CMob>> NpcContainer_t;

	//returns all connections
	ConnectionContainer_t		getAllConnections();
	//makes a client join the session
	void				joinSession(const handlePtr connection,std::string sessionId);
	//makes a client leave the session
	void				leaveSession(std::string sessionId);
	//returns if a client (session id) is already connected to the server
	bool				containsSessionId(std::string sessionId);

	//sends a string-packet to everyone in the session

	void				sendEveryone(std::string str) const;
	//sends a string-packet to a userID
	void				sendTo(std::string str, std::string sessionId);

	const handlePtrIt	getHandler(id_t id);
	std::shared_ptr<CMob> getMob(id_t id);

	void addMob(std::shared_ptr<CMob> m);
	void removeMob(id_t id);
	const size_t getMobCount();
	const id_t getMobHighestId();
private:
	ConnectionContainer_t m_connections;
	NpcContainer_t m_NPCs;
	id_t m_highestId;

	//checks if there are already IDs connected and if they do they get erased so only one Handler with only one ID is there
	void clearOldSession(handlePtr connection);
};
#endif