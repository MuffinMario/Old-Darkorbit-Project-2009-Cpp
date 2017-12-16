#ifndef HANDLER_H
#define HANDLER_H
#include <memory>
#include <atomic>
#include <set>
#include "Defines.h"
#include "MovementManager.h"
#include "Map.h"
#include "Player.h"

class Handler
{
	/* TO DO HERE: make atomic variables when going to make the damage calculations (HP/SHD thread-safe)*/
protected:
	//IN PLAYER CLASS/OR MAKE ONE NOW U NOOB
	id_t id{ 0 };
	PlayerInfo player;


	//positionmanager
	std::shared_ptr<MovementManager> mm;
	Map currentMap;

public:

	Handler() : currentMap(0), mm(nullptr) {  }

	virtual ~Handler() { }

	std::atomic<bool>	isAttacking = false;
	std::atomic<bool>	isInSMBCooldown = false;
	std::atomic<bool>	hasISH = false;
	bool				isJumping = false;

	id_t		getID()		const { return id; }
	pos_t		getX()		const { return mm->get_current_position_x(); }
	pos_t		getY()		const { return mm->get_current_position_y(); }
	map_t		getMapId()	const { return currentMap.getMapId(); }
	health_t	getHP()		const { return player.hp; }
	health_t	getSHD()	const { return player.shd; }
	health_t	getMaxHP()	const { return player.maxhp; }
	health_t	getMaxSHD() const { return player.maxshd; }


	virtual void receiveDamagePure(damage_t dmg, id_t from) = 0;
	virtual void receiveDamageHP(damage_t dmg, id_t from) = 0;
	virtual void receiveDamageSHD(damage_t dmg, id_t from) = 0;
	virtual void die() = 0;

	virtual void sendPacket(std::string str) = 0;
};

typedef boost::shared_ptr<Handler> handlePtr;
typedef std::set<handlePtr>::iterator handlePtrIt;

#endif