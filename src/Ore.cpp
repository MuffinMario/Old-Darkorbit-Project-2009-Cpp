#include "Ore.h"

extern CFileWriter g_filewrite;
extern CDBUpdater g_database_update;

void COre::rewardPlayer(id_t playerid)
{
	std::string oreType = (m_oretype == 1 ?
		"prometium" : (m_oretype == 2 ?
			"endurium" : (m_oretype == 3 ?
				"terbium" : "")));
	if (oreType == "") return;

	std::string query = 
		"UPDATE cuentas "
		"SET " + oreType + " = " + oreType + " + 1"
		"WHERE id = " + to_string(playerid);
	handlePtr user = m_session.get().getHandler(playerid);
	try {
		g_database_update.queryUpdateRaw(
			query
		);
	}
	catch(const mysql_exception& me)
	{
		warncout << "User " << playerid << " mysql_exception @COre::rewardPlayer : " << me.what() << cendl;
		user->disconnectUser();
	}
}

COre::COre(id_t oretype, id_t id, pos_t x, pos_t y, std::reference_wrapper<CSession> session)
	: ICollectable(id, x, y, session)
{
	m_oretype = oretype;
}

void COre::spawn(id_t to)
{
	handlePtr player = m_session.get().getHandler(to);
	if (player)
	{
		std::string createString = m_pm.createOre(m_id, m_oretype, m_pos.first, m_pos.second);
		player->sendPacket(createString.c_str());
	}
}

void COre::spawn()
{
	std::string createString = m_pm.createOre(m_id, m_oretype, m_pos.first, m_pos.second);
	m_session.get().sendEveryone(createString);
}

void COre::collect(id_t playerid)
{
	handlePtr player = m_session.get().getHandler(playerid);
	if (!player) return;

	if (verifyUserPosition(player))
	{
		rewardPlayer(playerid);

		remove();
	}
	else
	{
		/* USER IS NOT IN CORRECT OFFSET TO BONUS BOX */
		g_filewrite.writemore("User " + to_string(m_id) + " is " + to_string(std::make_pair(player->getX(), player->getY())) + " and ore is " + to_string(m_pos) + ".", "Ore");
	}
}
