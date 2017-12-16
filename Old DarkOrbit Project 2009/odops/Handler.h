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
class IHandler
{
	/* TO DO HERE: make atomic variables when going to make the damage calculations (HP/SHD thread-safe)*/
protected:
	id_t m_id{ 0 };
	std::string m_sessionId{ "" };
	CPlayerInfo m_player;


	//positionmanager
	std::shared_ptr<CMovementManager> m_mm;
	CMap m_currentMap;

	std::atomic_llong m_last_repair_prevention{ 0 };
	std::atomic_bool m_blogout_cancel{ 0 };
	std::atomic_llong m_last_logout_time{ 0 };
public:

	IHandler() : m_mm(nullptr) {  }

	virtual ~IHandler() { }

	std::atomic<bool>	m_pbIsAttacking{ false };
	std::atomic<bool>	m_pbIsInSMBCooldown{false};
	std::atomic<bool>	m_pbHasISH{ false };
	bool				m_pbIsJumping{ false };

	id_t		getID()		const { return m_id; }
	pos_t		getX()		const { return m_mm->get_current_position_x(); }
	pos_t		getY()		const { return m_mm->get_current_position_y(); }
	map_t		getMapId()	const { return m_currentMap.getMapId(); }
	health_t	getHP()		const { return m_player.hp; }
	health_t	getSHD()	const { return m_player.shd; }
	health_t	getMaxHP()	const { return m_player.maxhp; }
	health_t	getMaxSHD() const { return m_player.maxshd; }
	void updateRepairPrevent() { m_last_repair_prevention = getTimeNow() ; }
	void updateLogoutTime() { m_last_logout_time = getTimeNow(); }
	void toggleLogoutCancel(bool yesorno) { m_blogout_cancel = yesorno; }


	virtual damage_t receiveDamagePure(damage_t dmg) = 0;
	virtual damage_t receiveDamageHP(damage_t dmg) = 0;
	virtual damage_t receiveDamageSHD(damage_t dmg) = 0;
	virtual void die() = 0;

	virtual bool sendPacket(std::string str) = 0;
};

typedef std::shared_ptr<IHandler> handlePtr;
typedef std::map<std::string,handlePtr>::iterator handlePtrIt;

#endif