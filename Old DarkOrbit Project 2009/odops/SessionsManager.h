#ifndef SESSIONSMANAGER_H
#define SESSIONSMANAGER_H
#include <boost\shared_ptr.hpp>
#include <set>
#include "Handler.h"
#include "Defines.h"

class SessionsManager
{
	//std::set because of the performance on iterating through it, for more information ... use the internet i guess
	std::set<handlePtr> connections;
public:
	//returns all connections
	std::set<handlePtr> getAllConnections();
	//makes a client join the session
	void				joinSession(const handlePtr connection);
	//makes a client leave the session
	void				leaveSession(handlePtr connection);

	//sends a string-packet to everyone in the session

	void				sendEveryone(std::string str, const handlePtr connection) const;
	//sends a string-packet to a userID
	void				sendTo(std::string str, id_t userID);

	const handlePtrIt	getHandler(id_t id);
private:
	//checks if there are already IDs connected and if they do they get erased so only one Handler with only one ID is there
	void clearOldSession(handlePtr connection);
};
#endif