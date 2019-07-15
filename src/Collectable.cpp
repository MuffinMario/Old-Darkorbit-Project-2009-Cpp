#include "Collectable.h"

bool ICollectable::verifyUserPosition(handlePtr player)
{
	/* Distance user from box on collecting */
	static const pos_t solidOffsetX = 15;
	static const pos_t solidOffsetY = -70;

	pos_t offsetX = player->getX() - m_pos.first;
	pos_t offsetY = player->getY() - m_pos.second;

	/* Accepting distance from 10 - 20 / -65 - -75*/
	return (std::abs(offsetX - solidOffsetX) < 10 &&
			std::abs(offsetY - solidOffsetY) < 10);
}

CSession::Collectables_t::iterator ICollectable::remove()
{
	m_session.get().sendEveryone(m_pm.removeLoot(m_id));
	return m_session.get().removeCollectable(m_id);
}
