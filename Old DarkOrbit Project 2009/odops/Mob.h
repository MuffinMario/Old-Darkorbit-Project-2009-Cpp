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
//forward sessions
class CSessionsManager;
class CMob : public CMobWrapper
{
public:

	id_t id;


	id_t selectedId;
	id_t firstPlayerId;
	long long lastTimeShotByFirstPlayer = 0LL;
	long long waitForNextMoveTime = 0LL;
	bool isAttacking;

	CPacketComposer pm;
	CMovementManager mm;
	CSessionsManager* session;

	//default constructor for later initalisation
public:
	CMob() : mm(0, 0, 0) { }
	CMob(id_t mobid, CMobWrapper wrapper, Position_t pos, CSessionsManager* session);
	// default public constructor that also sends a spawn packet
	CMob(id_t mobid, CMobWrapper::Ship shipinfo, username_t name, CMobWrapper::Loot loot, unsigned short isAggressive, Position_t pos, CSessionsManager* session);
	//returns the time it takes in ms
	void move(pos_t x, pos_t y);
	void attack(id_t uid);
	void abort();
	void spawn(id_t user_id);
	void generateRandomWaitingTime(long long min, long long max);
	// min on 0 ms
	inline void generateRandomWaitingTime(long long max);
	// determines the time the npc is going to move (time since epoch)
	long long getNextMovingTime() { return this->waitForNextMoveTime; }
	id_t getId();
	Position_t getPosition();
	void setSession(CSessionsManager* session);

	damage_t receiveDamagePure(damage_t dmg);
	damage_t receiveDamageHP(damage_t dmg);
	damage_t receiveDamageSHD(damage_t dmg);
	void die();

	static CMobWrapper getMobInfoFromDB(shipid_t alienship_id);
	static std::vector<std::shared_ptr<CMob>> generateMobsForMap(map_t mapid);
	//TODO: Future me: Is this necessary?
	bool attacking();
};
#endif //__MOB_H__