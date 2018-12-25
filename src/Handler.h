#ifndef HANDLER_H
#define HANDLER_H
#include <memory>
#include <atomic>
#include <chrono>
#include <set>
#include "Defines.h"
#include "MovementManager.h"
#include "Player.h"
#include "Map.h"
// All default ammo every player has
struct Ammo
{
	ammo_t m_x1_LCB10;
	ammo_t m_x2_MCB25;
	ammo_t m_x3_MCB50;
	ammo_t m_x4_LCB50;
	ammo_t m_x5_SAB;
	rocket_t m_r1_R310;
	rocket_t m_r2_PLT2025;
	rocket_t m_r3_PLT2021;
	rocket_t m_r4_WIZ;
	ammo_t m_mines;
};
class IHandler
{
	/* TO DO HERE: make atomic variables when going to make the damage calculations (HP/SHD thread-safe)*/
protected:
	id_t m_id{ 0 };
	std::string m_sessionId{ "" };
	CPlayerInfo m_player;
	Ammo m_ammo;
	CMap m_currentMap;

	//positionmanager
	std::shared_ptr<CMovementManager> m_mm;

	std::atomic_llong m_last_repair_prevention{ 0 };
	std::atomic_llong m_last_shield_prevention{ 0 };
	std::atomic_llong m_last_naz_prevention{ 0 };
	std::atomic_bool m_blogout_cancel{ 0 };
	std::atomic_llong m_last_logout_time{ 0 };
	std::atomic<bool>	m_bIsRepairing = false;
public:

	IHandler() : m_mm(nullptr), m_player{}, m_ammo{} {  }

	virtual ~IHandler() { }

	std::atomic<bool>	m_pbIsAttacking{ false };
	std::atomic<bool>	m_pbIsInSMBCooldown{ false };
	std::atomic<bool>	m_pbHasISH{ false };
	std::atomic<bool>	m_pbIsInvisible{ false };
	bool				m_pbIsJumping{ false };


	bool		isRepairing() const { return m_bIsRepairing; }
	void		setRepairing(bool rep) { m_bIsRepairing = rep; }

	id_t		getID()		const { return m_id; }
	pos_t		getX()		const { return m_mm->get_current_position_x(); }
	pos_t		getY()		const { return m_mm->get_current_position_y(); }
	Position_t	getPos()	const { return m_mm->get_current_position(); }
	map_t		getMapId()	const { return m_currentMap.getMapId(); }
	health_t	getHP()		const { return m_player.hp; }
	shield_t	getSHD()	const { return m_player.shd; }
	health_t	getMaxHP()	const { return m_player.maxhp; }
	shield_t	getMaxSHD() const { return m_player.maxshd; }
	shield_t	getShieldRegen() const { return m_player.maxshd * 0.04; }
	credits_t	getCredits() const { return m_player.cred; }
	uri_t		getUri() const { return m_player.uri; }
	jackpot_t	getJP() const { return m_player.jackpot; }
	exp_t		getEP() const { return m_player.exp; }
	hon_t		getHonor() const { return m_player.hon; }
	level_t		getLevel() const { return m_player.lvl; }
	factionid_t	getFaction() const { return m_player.fractionid; }
	health_t	getRepBotHPRegen() const { return 10000; } /* TODO <- ADD VARIETY!!!!!*/

	virtual shield_t addSHD(shield_t shd) = 0;
	virtual health_t addHP(health_t hp) = 0;
	credits_t	addCredits(credits_t c) { return (m_player.cred += c); }
	uri_t		addUri(uri_t u) { return (m_player.uri += u); }
	jackpot_t	addJP(jackpot_t f) { return (m_player.jackpot += f); }
	exp_t		addEP(exp_t e) { 
		m_player.exp += e;
		if (e != 0) 
			onEPChange();
		return m_player.exp; 
	}
	hon_t		addHonor(hon_t h) { return (m_player.hon += h); }
	level_t		addLevel(level_t l) { return (m_player.lvl += l); }

	credits_t	reduceCredits(credits_t c) { return (m_player.cred -= c); }
	uri_t		reduceUri(uri_t u) { return (m_player.uri -= u); }
	exp_t		reduceEP(exp_t e) { return (m_player.exp -= e); }
	hon_t		reduceHonor(hon_t h) { return (m_player.hon -= h); }
	level_t		reduceLevel(level_t l) { return (m_player.lvl -= l); }

	void updateRepairPrevent() { m_last_repair_prevention = getTimeNow(); m_bIsRepairing = false; }
	void updateShieldPrevent() { m_last_shield_prevention = getTimeNow(); }
	void updateNAZPrevent() { m_last_naz_prevention = getTimeNow(); }
	void updateLogoutTime() { m_last_logout_time = getTimeNow(); }
	long long getRepairPreventTime() { return m_last_repair_prevention; }
	long long getLogoutTime() { return m_last_logout_time; }
	long long getShieldPreventTime() { return m_last_shield_prevention; }
	long long getNAZPreventTime() { return m_last_naz_prevention; }

	
	void toggleLogoutCancel(bool yesorno) { m_blogout_cancel = yesorno; }


	virtual void checkForObjectsToInteract() = 0;

	virtual void onEPChange() = 0;
	virtual damage_t receiveDamagePure(damage_t dmg) = 0;
	virtual damage_t receiveDamageHP(damage_t dmg) = 0;
	virtual damage_t receiveDamageSHD(damage_t dmg) = 0;
	virtual void die() = 0;
	virtual void disconnectUser() = 0;
	virtual void suspendThreads() = 0;


	virtual bool sendPacket(const char* str) = 0;
	virtual bool sendPacket(const std::string& str) = 0;
	virtual bool sendPacketSynchronous(std::string& str) = 0;
	virtual bool sendPacketSynchronous(const char* str) = 0;
};

typedef std::shared_ptr<IHandler> handlePtr;
typedef std::map<std::string,handlePtr>::iterator handlePtrIt;

#endif