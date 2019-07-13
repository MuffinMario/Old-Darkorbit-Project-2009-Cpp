#include "BonusBox.h"

extern CDBUpdater g_database_update;
extern CFileWriter g_filewrite; // log if user collected somewhere else


const std::vector<credits_t> CBonusBox::cr_p = { 200,500,1000 };
const std::vector<uri_t> CBonusBox::u_p = { 20,50,100 };
const std::vector<jackpot_t> CBonusBox::jp_p = { 0.25f,0.5f,1.0f };
const std::vector<extraenergy_t> CBonusBox::ee_p = { 1 };
const std::vector<ammo_t> CBonusBox::aax1_p = { 10,20,50 };
const std::vector<ammo_t> CBonusBox::aax23SAB_p = { 5,10,20 };

CBonusBox::CBonusBox(id_t id, pos_t x, pos_t y, loottype_t loottype, double amount, std::reference_wrapper<CSession> session)
	: ICollectable(id, x, y, session)
{
	m_type = BONUS_BOX_DEFAULT_COLLECTABLE_ID;

	switch (loottype)
	{
	case LT_CRED:
	{
		credits_t credits = static_cast<credits_t>(amount);
		m_bb_content.m_credits = credits;
	}
		break;
	case LT_URI:
	{
		uri_t uri = static_cast<uri_t>(amount);
		m_bb_content.m_uridium = uri;
	}
		break;
	case LT_JP:
	{
		jackpot_t jp = static_cast<jackpot_t>(amount);
		m_bb_content.m_jackpot = jp;
	}
		break;
	case LT_AMMO_X1:
	case LT_AMMO_X2:
	case LT_AMMO_X3:
	case LT_AMMO_X4:
	case LT_AMMO_SAB:
	{
		ammo_t ammo = static_cast<ammo_t>(amount);
		lasertype_t ammotype = loottype - (LT_AMMO_X1)+1;
		//test
		m_bb_content.m_ammo.m_ammo_type = ammotype;
		m_bb_content.m_ammo.m_ammo_amount = ammo;
	}
		break;
	case LT_EE:
	{
		extraenergy_t ee = static_cast<extraenergy_t>(amount);
		m_bb_content.m_extraenergy = ee;
	}
		break;
	default:
		dcout << "WRONG LT IN BonusBox.cpp: " << loottype << cendl;
	}
}

void CBonusBox::rewardPlayer(id_t playerid)
{
	handlePtr player = m_session.get().getHandler(playerid);
	if (player == nullptr) return;
	// If everything thought out right, sql injection is impossible due to no user access input and only numbers
	std::string sqlSET = "UPDATE cuentas SET ";
	size_t rewards = 0;
	double multiplier = 15; // 15x
	if (m_bb_content.m_ammo.m_ammo_amount != 0)
	{
		++rewards;
		std::string ammoType = "";

		switch (m_bb_content.m_ammo.m_ammo_type)
		{
		case 1:
			ammoType = "LCB10";
			break;
		case 2:
			ammoType = "MCB25";
			break;
		case 3:
			ammoType = "MCB50";
			break;
		case 4: // high likely unused until event
			ammoType = "UCB50";
			break;
		case 5:
			ammoType = "SAB";
			break;
		default:
			ammoType = "LCB10";
		}
		//TODO: player->addAmmo(ammoType, m_bb_content.m_ammo.m_ammo_amount);
		if (ammoType != "")
		{
			sqlSET += ammoType + " = " + ammoType + " + " + to_string(m_bb_content.m_ammo.m_ammo_amount);
		}
		player->sendPacket(m_pm.receiveLoot("BAT", { (long long)m_bb_content.m_ammo.m_ammo_type,(long long)(m_bb_content.m_ammo.m_ammo_amount*multiplier) }));
	}
	if (m_bb_content.m_credits != 0)
	{
		if (rewards++ > 0) sqlSET += ", ";
		sqlSET += "creditos = creditos + " + to_string(m_bb_content.m_credits);

		long long plus = m_bb_content.m_credits * multiplier;
		long long total = player->addCredits(plus);

		player->sendPacket(m_pm.receiveLoot("CRE", { plus,total }));
	}
	if (m_bb_content.m_uridium != 0)
	{
		if (rewards++ > 0) sqlSET += ", ";
		sqlSET += "uridium = uridium + " + to_string(m_bb_content.m_uridium);

		long long plus = m_bb_content.m_uridium;
		long long total = player->addUri(plus);

		player->sendPacket(m_pm.receiveLoot("URI", { plus,total }));
	}
	if (m_bb_content.m_jackpot != 0.0f)
	{
		if (rewards++ > 0) sqlSET += ", ";
		sqlSET += "jackpot = jackpot + " + to_string(m_bb_content.m_jackpot);

		float plus = m_bb_content.m_jackpot;
		float total = player->addJP(plus);

		player->sendPacket(m_pm.receiveLoot("JPE", { plus,total }));
	}
	if (m_bb_content.m_extraenergy != 0)
	{
		if(rewards++ > 0) sqlSET += ", ";
		sqlSET += "energia_extra = energia_extra + " + to_string(m_bb_content.m_extraenergy);
		for(extraenergy_t i = 0; i < m_bb_content.m_extraenergy ;i++)
			player->sendPacket(m_pm.receiveLoot("XEN"));
	}
	if (rewards > 0)
	{
		handlePtr user = m_session.get().getHandler(playerid);
		try {
			sqlSET += " WHERE id = " + to_string(playerid);
			g_database_update.queryUpdateRaw(
				sqlSET
			);
		}
		catch (mysql_exception& me)
		{
			warncout << "User " << playerid << " mysql_exception @CBonusBox::rewardPlayer : " << me.what() << cendl;
			user->disconnectUser();
		}
	}
	// possible packets. CRE URI XEN JP BAT

}
void CBonusBox::collect(id_t playerid)
{
	handlePtr player = m_session.get().getHandler(playerid);
	if (player == nullptr) return;
	map_t mapid = player->getMapId();
	/*
	//
	// Example output:
	//		user: 10678/6146
	//		box	  10663/6216
	//		delta: 15/-70 -> it was always 15/-70
	dcout << "user: " << player->getX() << "/" << player->getY() << "\n"
		<< "box: " << m_pos.first << "/" << m_pos.second << "\n"
		<< "delta: " << player->getX() - m_pos.first << "/" << player->getY() - m_pos.second << cendl;*/
	if (verifyUserPosition(player))
	{
		rewardPlayer(playerid);

		// Remove from session
		remove();

		m_session.get().addCollectable(generateNewBonusBox(m_session.get().generateNewCollectableId(), mapid, m_session));

	}
	else
	{
		/* USER IS NOT IN CORRECT OFFSET TO BONUS BOX */
		g_filewrite.writemore("User " + to_string(m_id) + " is " + to_string(std::make_pair(player->getX(), player->getY())) + " and box is " + to_string(m_pos) + ".", "BBox");
	}
}

void CBonusBox::spawn()
{
	std::string createString = m_pm.createLoot(m_id, m_type, m_pos.first, m_pos.second);
	m_session.get().sendEveryone(createString);
}

std::shared_ptr<CBonusBox> CBonusBox::generateNewBonusBox(id_t id, map_t mapid,std::reference_wrapper<CSession> session)
{
	/* Lazy lambda */
	static auto isBigMap = [](map_t mid) { return mid == 16 || mid == 29; };
	/* Set values */
	size_t map_width = isBigMap(mapid) ? CMap::MAPSIZE_BIG_X : CMap::MAPSIZE_NORMAL_X;
	size_t map_height = isBigMap(mapid) ? CMap::MAPSIZE_BIG_Y : CMap::MAPSIZE_NORMAL_Y;
	loottype_t lt = loottype_t::LT_CRED;
	double amount = 0.0;
	double basic_multiplier = 1.0;
	
	if (mapid >= 13 && mapid <= 15)
	{
		basic_multiplier = 1.5;
	}

	/* Generate loottype + lootamount for normal maps */
	if (mapid != 42)
	{
		int max = CBonusBox::loottype_t::LT_LT_MAX - 1;
		lt = static_cast<CBonusBox::loottype_t>(random<int>(max)); // re-cast because integral type
		if (lt == CBonusBox::loottype_t::LT_AMMO_X4)
		{
			// we dont generate x4 instead we distribute all x4s to x1-3 and SAB
			int newlt = random<int>(0, 3);
			if (newlt == 3) {
				newlt++;
			}
			lt = static_cast<CBonusBox::loottype_t>(CBonusBox::loottype_t::LT_AMMO_X1 + newlt);
		}
		switch (lt)
		{
		case CBonusBox::loottype_t::LT_CRED:
			amount = cr_p.at(random<size_t>(cr_p.size() - 1));
			break;
		case CBonusBox::loottype_t::LT_URI:
			amount = u_p.at(random<size_t>(u_p.size() - 1));
			break;
		case CBonusBox::loottype_t::LT_AMMO_X1:
			amount = aax1_p.at(random<size_t>(aax1_p.size() - 1));
			break;
		case CBonusBox::loottype_t::LT_AMMO_X2:
		case CBonusBox::loottype_t::LT_AMMO_X3:
		case CBonusBox::loottype_t::LT_AMMO_SAB:
			amount = aax23SAB_p.at(random<size_t>(aax23SAB_p.size() - 1));
			break;
		case CBonusBox::loottype_t::LT_JP:
			amount = jp_p.at(random<size_t>(jp_p.size() - 1));
			break;
		case CBonusBox::loottype_t::LT_EE:
			amount = ee_p.at(random<size_t>(ee_p.size() - 1));
			break;
		default:
			dcout << "GENERATING CBonusBox WITH UNDEFINED LOOTTYPE: " << lt << cendl;
		}
		if (lt != CBonusBox::loottype_t::LT_EE)
		{
			amount *= basic_multiplier;
		}
		else
		{
			amount = std::round(amount*basic_multiplier);
		}
	}
	else
	{
		lt = loottype_t::LT_AMMO_X4;
		amount = 1000;
	}
	/* return new bonusbox */
	return std::make_shared<CBonusBox>(id, random<pos_t>(0, map_width), random<pos_t>(0, map_height), lt, amount, std::ref(session));
}

std::vector<std::shared_ptr<CBonusBox>> CBonusBox::generateBonusBoxes(map_t mapid,std::reference_wrapper<CSession> session)
{
	std::vector<std::shared_ptr<CBonusBox>> bbs = {};
	if (mapid != 16)
	{
		size_t count = 200;
		if (mapid == 42 || mapid == 29) // 29 because size 42 because rare map
		{
			count *= 2;
		}
		for (size_t i = 1; i <= count; i++)
		{
			bbs.push_back(generateNewBonusBox(session.get().generateNewCollectableId(), mapid, session));
		}
	}
	return bbs;
}

void CBonusBox::spawn(id_t to)
{
	handlePtr player = m_session.get().getHandler(to);
	if (player)
	{
		std::string createString = m_pm.createLoot(m_id, m_type, m_pos.first, m_pos.second);
		player->sendPacket(createString);
	}
}
