#include "ResourceBox.h"

extern CFileWriter g_filewrite;
extern CDBUpdater g_database_update;

void CResourceBox::rewardPlayer(id_t playerid)
{
	handlePtr player = m_session.get().getHandler(playerid);
	if (player == nullptr) return;
	std::string prefix = "UPDATE cuentas SET ";
	std::string content = "";
	std::string suffix = "WHERE id = " + to_string(playerid);
	cargo_t cargoLeftOnPlayer = player->getCargoSpaceLeft();
	bool notEnoughSpace = false;
	const auto appendStringIfLootAboveZero = [&cargoLeftOnPlayer,&notEnoughSpace](std::string& s, const char* colname, ore_t& ore)
	{
		ore_t oreAmount = ore;
		if (strcmp(colname, "xenomit") != 0) // xenomit is the only exception of not taking any space 
		{
			if (notEnoughSpace) return s;

			if (cargoLeftOnPlayer - oreAmount < 0) {
				notEnoughSpace = true;
				oreAmount = cargoLeftOnPlayer; // amount received is inventory left
			}
			cargoLeftOnPlayer -= oreAmount;
		}
		ore -= oreAmount;
		if (oreAmount > 0)
		{
			std::string col = colname;
			return s + 
				((s=="")? col :"," + col) +  // if string not empty ",endurium" instead of "endurium"
				" = " + col + "+" + to_string(oreAmount) + " ";
		}
		else
		{
			return s;
		}
	};
	auto originalLoot = m_lootOres;
	CMobWrapper::Loot deltaLoot{ 0 };
	content = appendStringIfLootAboveZero(content, "promerium", m_lootOres.o3_promerium);
	content = appendStringIfLootAboveZero(content, "duranium", m_lootOres.o2_duranium);
	content = appendStringIfLootAboveZero(content, "prometid", m_lootOres.o2_prometid);
	content = appendStringIfLootAboveZero(content, "xenomit", m_lootOres.o4_xenomit);
	content = appendStringIfLootAboveZero(content, "terbium", m_lootOres.o1_terbium);
	content = appendStringIfLootAboveZero(content, "endurium", m_lootOres.o1_endurium);
	content = appendStringIfLootAboveZero(content, "prometium", m_lootOres.o1_prometium);

	deltaLoot.o1_prometium = originalLoot.o1_prometium - m_lootOres.o1_prometium;
	deltaLoot.o1_endurium = originalLoot.o1_endurium - m_lootOres.o1_endurium;
	deltaLoot.o1_terbium = originalLoot.o1_terbium - m_lootOres.o1_terbium;

	deltaLoot.o2_prometid = originalLoot.o2_prometid - m_lootOres.o2_prometid;
	deltaLoot.o2_duranium = originalLoot.o2_duranium - m_lootOres.o2_duranium;

	deltaLoot.o3_promerium = originalLoot.o3_promerium - m_lootOres.o3_promerium;

	deltaLoot.o4_xenomit = originalLoot.o4_xenomit- m_lootOres.o4_xenomit;

	/* user actually gets something */
	if (content != "")
	{
		std::string sqlCommand = prefix + content + suffix;

		/* Update database with command */
		g_database_update.queryUpdateRaw(
			sqlCommand
		);
		/* Send user receive message */
		player->sendPacket(m_pm.receiveLoot("CAR", {
			(long long)deltaLoot.o1_prometium, (long long)deltaLoot.o1_endurium, (long long)deltaLoot.o1_terbium
			, (long long)deltaLoot.o4_xenomit
			, (long long)deltaLoot.o2_prometid , (long long)deltaLoot.o2_duranium
			, (long long)deltaLoot.o3_promerium }));
		player->updateCargo();
	}
	if (notEnoughSpace)
		player->sendPacket("0|y|BTB"); // I guess BOX TOO BIG is appropriate
	if (boxIsPrivate() && m_belongsTo != playerid)
	{
		hon_t minusHon = 100;
		hon_t newhon = player->reduceHonor(minusHon);
		player->sendPacket(m_pm.receiveLoot("HON", { -((long long)minusHon),(long long)newhon }));
	}

}

bool CResourceBox::userGetsOrange(id_t userId, factionid_t company)
{
	return userId == m_belongsTo || 
		company != m_belongsToCompany;
}

bool CResourceBox::userGetsOrangeHandle(handlePtr player)
{
	return userGetsOrange(player->getID(), player->getFaction());
}

bool CResourceBox::userGetsBlueHandle(handlePtr player)
{
	return !userGetsOrange(player->getID(), player->getFaction());
}

bool CResourceBox::boxIsPrivate()
{
	return m_belongsTo != 0 && 
		m_belongsToSeconds != 0;
}

CResourceBox::CResourceBox(const CMobWrapper::Loot& ores, id_t id, pos_t x, pos_t y, std::reference_wrapper<CSession> session, id_t privateForId)
	: ICollectable(id,x,y,session)
{
	m_belongsTo = privateForId;
	if (m_belongsTo == 0)
	{
		/* FOR EVERYONE */
		m_type = RESOURCE_BOX_DEFAULT_COLLECTABLE_ID;
		m_belongsToSeconds = 0LL;
		m_belongsToCompany = 0U;
	}
	else
	{
		/* FOR SOMEONE FIRST */
		m_type = RESOURCE_BOX_PRIVATE_COLLECTABLE_ID;
		m_belongsToSeconds = RESOURCE_BOX_PRIVATE_SECONDS;
		handlePtr playerPtr = m_session.get().getHandler(m_belongsTo);
		if (playerPtr) 
		{
			m_belongsToCompany = playerPtr->getFaction();
		}
	}
	m_lootOres = ores;
}

void CResourceBox::spawn(id_t to)
{
		handlePtr player = m_session.get().getHandler(to);
		if (player)
		{
			/* If resource box is private and timer is running */
			if (boxIsPrivate())
			{
				
				if (userGetsOrange(player->getID(),player->getFaction()))
				{
					/* Collectable box for private id and enemigo */
					std::string createString = m_pm.createLoot(m_id, RESOURCE_BOX_DEFAULT_COLLECTABLE_ID, m_pos.first, m_pos.second);
					player->sendPacket(createString);
				}
				else
				{
					/* Blue "DONT COLLECT" box for everyone else - id */
					std::string createString = m_pm.createLoot(m_id, RESOURCE_BOX_PRIVATE_COLLECTABLE_ID, m_pos.first, m_pos.second);
					player->sendPacket(createString);
				}
			}
			/* If resource box is not private anymore */
			else
			{
				std::string createString = m_pm.createLoot(m_id, RESOURCE_BOX_DEFAULT_COLLECTABLE_ID, m_pos.first, m_pos.second);
				player->sendPacket(createString);
			}
		}
}

void CResourceBox::spawn()
{
	if (boxIsPrivate())
	{
		/* Send everyone blue ressource except id m_belongsTo and enemy */
		std::string createOrange = m_pm.createLoot(m_id, RESOURCE_BOX_DEFAULT_COLLECTABLE_ID, m_pos.first, m_pos.second);
		std::string createBlue = m_pm.createLoot(m_id, RESOURCE_BOX_PRIVATE_COLLECTABLE_ID, m_pos.first, m_pos.second);

		// binds the functor to the current resource box class.
		std::function<bool(handlePtr)> getsBlue = std::bind(&CResourceBox::userGetsBlueHandle, this, std::placeholders::_1);
		std::function<bool(handlePtr)> getsOrange = std::bind(&CResourceBox::userGetsOrangeHandle, this, std::placeholders::_1);
		m_session.get().sendEveryoneBut(createBlue, getsBlue);
		m_session.get().sendEveryoneBut(createOrange, getsOrange);
	}
	else
	{
		std::string createOrange = m_pm.createLoot(m_id, RESOURCE_BOX_DEFAULT_COLLECTABLE_ID, m_pos.first, m_pos.second);
		m_session.get().sendEveryone(createOrange);
	}
}

void CResourceBox::collect(id_t playerid)
{
	handlePtr player = m_session.get().getHandler(playerid);
	if (player == nullptr) return;
	
	if (verifyUserPosition(player))
	{
		// this is not thread safe?
		rewardPlayer(playerid);

		// Remove from session
		if(empty())
			remove();
	}
	else
	{
		/* USER IS NOT IN CORRECT OFFSET TO BONUS BOX */
		g_filewrite.writemore("User " + to_string(m_id) + " is " + to_string(std::make_pair(player->getX(), player->getY())) + " and box is " + to_string(m_pos) + ".", "RBox");
	}
}

bool CResourceBox::empty()
{
	return m_lootOres.o1_prometium <= 0 &&
		m_lootOres.o1_endurium <= 0 &&
		m_lootOres.o1_terbium <= 0 &&
		
		m_lootOres.o2_prometid <= 0 &&
		m_lootOres.o2_duranium <= 0 &&
		
		m_lootOres.o3_promerium <= 0 &&
		
		m_lootOres.o4_xenomit <= 0;
}


void CResourceBox::onChangeToPublic()
{
	m_session.get().sendEveryoneBut(m_pm.removeLoot(m_id), m_belongsTo);
	m_session.get().sendEveryoneBut(m_pm.createLoot(m_id, RESOURCE_BOX_DEFAULT_COLLECTABLE_ID, m_pos.first, m_pos.second),m_belongsTo);
}
