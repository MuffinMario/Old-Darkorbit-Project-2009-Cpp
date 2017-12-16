#include "SessionsManager.h"

std::set<handlePtr> SessionsManager::getAllConnections()
{
	return connections;
}

void SessionsManager::joinSession(handlePtr connection)
{
	clearOldSession(connection);
	connections.insert(connection);
	std::cout << "joined, size: " << connections.size() << std::endl;
}
void SessionsManager::leaveSession(handlePtr connection)
{
	std::cout << "before left, size: " << connections.size() << std::endl;
	connections.erase(connection);
	std::cout << "left, size: " << connections.size() << std::endl;
}

//THIS DOES NOT PROVIDE SOLID CLEARANCE ON EVERY MAP; RECONSIDER THIS METHOD
void SessionsManager::clearOldSession(handlePtr connection)
{
	int userID = connection->getID();
	for (auto& it : connections) {
		if (userID == it->getID()) {
			connections.erase(it);
		}
	}
}
void SessionsManager::sendEveryone(std::string str, handlePtr connection) const
{
	if (connections.size() < 1) return;
	for (auto& it : connections) {
		if (it->getMapId() == connection->getMapId()) {
			it->sendPacket(str);
		}
	}
}

void SessionsManager::sendTo(std::string str, id_t userID) {
	auto it = getHandler(userID);
	if (it != connections.end())
		it->get()->sendPacket(str);
}


//returns the iterator to the first result of this id, maybe something else instead of an iterator since this is not what an iterator is ment to be used to
// WARNING: PLEASE CONSIDER ALWAYS DOING A IT != container.end() CHECK
const handlePtrIt SessionsManager::getHandler(id_t id) {
	return std::find_if(connections.begin(), connections.end(), [&id](const handlePtr& hndl) { return hndl->getID() == id; });

}
/*handlePtr SessionsManager::getHandleWeak(id_t id) {
for (auto it = connections.begin(); it != connections.end(); ++it) {
if (it->get()->getHP() == id) {
return *it;
}
}
throw std::exception(
}*/