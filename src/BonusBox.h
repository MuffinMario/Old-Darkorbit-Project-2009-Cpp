#pragma once
#ifndef BONUSBOX_H
#define BONUSBOX_H
#include "Collectable.h"
#include "Handler.h"
#include "DBUpdater.h"
#include "FileWriter.h"


/*
	normal maps:

	200/500/1000 cred
	20/50/100 uri
	10/20/50 x1
	5/10/20 x2/x3/SAB
	0.25/0.5/1.0 JPE
	1 EE

	battle maps:
	cred,uri,ammo,jpe * 1.5 
	2 EE

/* Implementation of a bonus box on a space map */
class CBonusBox : public ICollectable
{
	struct _loot { //potentially giving multiple things
		credits_t m_credits = 0;
		uri_t m_uridium = 0;
		jackpot_t m_jackpot = 0;
		struct _ammo_loot {
			lasertype_t m_ammo_type = 0;
			ammo_t m_ammo_amount = 0;
		} m_ammo;
		extraenergy_t m_extraenergy = 0;
	} m_bb_content;


	/* possible loot amounts for types */
	static const std::vector<credits_t> cr_p;
	static const std::vector<uri_t> u_p;
	static const std::vector<jackpot_t> jp_p;
	static const std::vector<extraenergy_t> ee_p;
	static const std::vector<ammo_t> aax1_p;
	static const std::vector<ammo_t> aax23SAB_p;

	void rewardPlayer(id_t player);
public:

	enum loottype_t{
		LT_CRED,
		LT_URI,
		LT_JP,
		LT_AMMO_X1,
		LT_AMMO_X2,
		LT_AMMO_X3,
		LT_AMMO_X4, //unused
		LT_AMMO_SAB,
		LT_EE,
		LT_LT_MAX
	};
	CBonusBox(id_t id, pos_t x, pos_t y,loottype_t loottype,double amount,std::reference_wrapper<CSession> session);

	void collect(id_t player) override;
	void spawn(id_t to) override;
	void spawn() override;

	static std::shared_ptr<CBonusBox> generateNewBonusBox(id_t id,map_t mapid, std::reference_wrapper<CSession> session);
	static std::vector<std::shared_ptr<CBonusBox>> generateBonusBoxes(map_t mapid, std::reference_wrapper<CSession> session);

	static const collectable_t BONUS_BOX_DEFAULT_COLLECTABLE_ID = 2;
	
};

#endif