#pragma once

#include <mutex>

#include "Defines.h"
#include "PacketComposer.h"
#include "Session.h"

class CSession;
class ICollectable
{

	std::mutex m__resource_mutex;
protected:
	CPacketComposer m_pm;
	collectable_t m_type;
	id_t m_id;
	Position_t m_pos; //immovable object
	std::reference_wrapper<CSession> m_session;

	bool verifyUserPosition(handlePtr player);

public:

	std::vector<std::shared_ptr<ICollectable>>::iterator remove();
	ICollectable(id_t id, pos_t x, pos_t y, std::reference_wrapper<CSession> session)
		:m_session(session), m_type(-1), m_pos(x, y)
	{
		m_id = id;
	}

	virtual ~ICollectable() {}

	virtual void spawn(id_t to) = 0;
	virtual void spawn() = 0;
	virtual void collect(id_t playerid) = 0;

	id_t getId() { return m_id; }
	Position_t getPos() { return m_pos; }
	collectable_t getType() { return m_type; }
};