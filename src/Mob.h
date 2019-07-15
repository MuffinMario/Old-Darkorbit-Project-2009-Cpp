//side note: I actually intended to inherit IHandler but I thought it would take too much time for nothing
#ifndef __MOB_H__
#define __MOB_H__
#include <vector>
#include <regex>
#include <random>
#include <map>
#include <mutex>
#include "Defines.h"
#include "PacketComposer.h"
#include "MovementManager.h"
#include "MobWrapper.h"
#include "Session.h"

//forward sessions
class CSession;
class CMob : public CMobWrapper
{
private:

	id_t m_id = 0;
	std::mutex m_mob_mutex;

	id_t m_selectedId = 0;

	id_t m_focusedPlayerId = 0;
	id_t m_belongToId = 0;
	long long m_lastTimeShotByBelongedPlayer = 0LL;

	long long m_waitForNextMoveTime = 0LL;
	bool m_isAttacking = false;
	bool m_greyed = false;
	CPacketComposer m_pm;
	CMovementManager m_mm;
	std::reference_wrapper<CSession> m_session;

	/* Maybe theres someone who makes 4x more damage? Maybe theres someone shooting while npc follows someone else? */
	void handleWhoAmIGonnaShoot(damage_t dmg, id_t from);
public:
	CMob() = delete;
	CMob(id_t mobid, const CMobWrapper& wrapper, Position_t pos, std::reference_wrapper<CSession> session);
	// default public constructor that also sends a spawn packet
	CMob(id_t mobid, const CMobWrapper::Ship& shipinfo, username_t name, const CMobWrapper::Loot& loot,
		const CMobWrapper::Reward& reward, unsigned short isAggressive, Position_t pos, 
		std::reference_wrapper<CSession> session);
	//returns the time it takes in ms
	void move(pos_t x, pos_t y);
	void attack(id_t uid);
	void abort();
	void spawn(id_t user_id);
	void spawn();
	void generateRandomWaitingTime(long long min, long long max);
	void setRealWaitingTime(long long val);
	// min on 0 ms
	inline void generateRandomWaitingTime(long long max);
	// determines the time the npc is going to move (time since epoch)
	long long getNextMovingTime() { return this->m_waitForNextMoveTime; }
	long long getLastTimeShotByBelongedPlayer() const { return this->m_lastTimeShotByBelongedPlayer; }
	void setLastTimeShotByBelongedPlayer(long long ltsbp) { this->m_lastTimeShotByBelongedPlayer = ltsbp; }


	bool isGreyed() { return m_greyed; }
	void grey() { m_greyed = true; }
	void ungrey() { m_greyed = false; }

	id_t getId();
	Position_t getPosition();
	void setSession(std::reference_wrapper<CSession> session);
	void setFocusToPlayer(id_t from);
	/* Gets the id of the player that the npc is following */
	id_t getFocusedPlayer();
	/* Gets the id of the player that the npc is rewarding on dead*/
	id_t getBelongedPlayer();
	void updateLootOwner(damage_t dmg,id_t id);
	damage_t receiveDamagePure(damage_t dmg, id_t from);
	damage_t receiveDamageHP(damage_t dmg,id_t from);
	damage_t receiveDamageSHD(damage_t dmg, id_t from);

	
	CSession::NpcContainer_t::iterator die();
	void rewardLoot();
	void dropResources();

	static CMobWrapper getMobInfoFromDB(shipid_t alienship_id);
	static std::vector<std::shared_ptr<CMob>> generateMobsForMap(map_t mapid,std::reference_wrapper<CSession> session);
	//TODO: Future me, Is this necessary?
	//	Future me: YES
	bool attacking();
};
#endif //__MOB_H__