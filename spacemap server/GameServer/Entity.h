#pragma once
#include "../common/Defines.h"
#include "SessionTickable.h"
class IEntity : public ISessionTickable{
public:
	/*
	*	Moves Entity to position x,y
	*/
	virtual void move(pos_t x, pos_t y) = 0;
	/*
	*	Attacks player id uid
	*/
	virtual void attack(id_t uid) = 0;
	/*
	*	Aborts attack	
	*/
	virtual void abort() = 0;
	/*
	*	Spawn for userid
	*/
	virtual void spawn(id_t user_id) = 0;
	/*
	*	Spawn for everyone in session
	*/
	virtual void spawn() = 0;
};