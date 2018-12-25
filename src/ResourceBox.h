#pragma once
#ifndef RESOURCEBOX_H
#define RESOURCEBOX_H

#include <iostream>
#include <mutex>

#include "Defines.h"
#include "Collectable.h"
#include "DBUpdater.h"
#include "FileWriter.h"
#include "MobWrapper.h" // contains "Loot" class

class CResourceBox : public ICollectable
{
	CMobWrapper::Loot m_lootOres;

	void rewardPlayer(id_t playerid);

	inline bool userGetsOrange(id_t userId, factionid_t company);
	/* Determine functions */
	bool userGetsOrangeHandle(handlePtr player);
	bool userGetsBlueHandle(handlePtr player);

public:
	id_t m_belongsTo = 0;
	factionid_t m_belongsToCompany = 0;
	long long m_belongsToSeconds = 0LL;
	long long m_existsSeconds = 60LL; /* 60 seconds until box dies RIP*/

	CResourceBox(const CMobWrapper::Loot& ores,id_t id, pos_t x, pos_t y, std::reference_wrapper<CSession> session, id_t privateForId);

	inline bool boxIsPrivate();
	void spawn(id_t to) override;
	void spawn() override;
	void collect(id_t playerid) override;

	void onChangeToPublic();

	static const collectable_t RESOURCE_BOX_DEFAULT_COLLECTABLE_ID = 1;
	static const collectable_t RESOURCE_BOX_PRIVATE_COLLECTABLE_ID = 0;
	static const long long RESOURCE_BOX_PRIVATE_SECONDS = 8LL;
};

#endif