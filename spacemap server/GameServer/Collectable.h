#pragma once

#include <mutex>

#include "../common/Defines.h"
#include "../common/PacketComposer.h"
#include "Session.h"
#include "SessionTickable.h"
class CSession;
class ICollectable : public ISessionTickable
{

	std::mutex m__resource_mutex;
protected:
	CPacketComposer m_pm;
	collectable_t m_type;
	id_t m_id;
	Position_t m_pos; //immovable object
	std::reference_wrapper<CSession> m_session;

	bool verifyUserPosition(handlePtr player);
	void spawnBoxDefault(id_t to);
	void spawnBoxDefault();
	void spawnOreDefault(id_t to);
	void spawnOreDefault();

public:

	CSession::Collectables_t::iterator remove();
	ICollectable(id_t id, pos_t x, pos_t y, std::reference_wrapper<CSession> session)
		:m_session(session), m_type(-1), m_pos(x, y)
	{
		m_id = id;
	}
	ICollectable(const ICollectable&) = default;
	ICollectable(ICollectable&&) = default;
	virtual ~ICollectable() {}

	virtual void spawn(id_t to) = 0;
	virtual void spawn() = 0;
	virtual void collect(id_t playerid) = 0;

	id_t getId() const noexcept { return m_id; }
	Position_t getPos() const noexcept { return m_pos; }
	collectable_t getType() const noexcept { return m_type; }

};