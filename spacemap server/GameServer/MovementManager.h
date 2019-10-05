#ifndef MovementManager_H
#define MovementManager_H
#include <chrono>
#include <iostream>
#include <atomic>
#define _USE_MATH_DEFINES
#include <cmath>
#include "../common/Defines.h"

//taken from http://www.highprogrammer.com/alan/windev/visualstudio.html
/*#define Stringize( L )     #L 
#define MakeString( M, L ) M(L)
#define $Line MakeString( Stringize, __LINE__ )
#define Reminder __FILE__ "(" $Line ") : warning C0: "


#pragma message (Reminder "Atomic included for testing in, remove if not needed anymore: " __FILE__)
#include <atomic>*/

///////////////////////////////////////////
//		"You cannot trust these clients anymore"
//	Thus we have to make this class. also to prevent cheating (f.e. speed hack via cheat engine) of course
//	this is actually pretty simple for the beginning:
//	Player logs in -> local Vec2 
//  Player moves -> planned Vec2
//  (Packet moves -> local & planned Vec2) <- remember that we won't use the local pos for now, later for maybe punishing in case of cheating
//  @SIDENOTE: This is my fourth try on doing a movementmanager, this time I used mspaint and it worked, I even started with a percentage calculation on positions, 
//        and now look what we got here (something not well working, but well working)
//  @WORKING:
//		- nearly everything
//  @NOT WORKING:
//		- ...except the mystical exponential speed lowerment of the client
//			'-> the further the way, the slower the client, and we simply don't know why
//				(see DoAction_30 this.speed = this.Tspeed / framerate / 0.97; )
//				2017:	changing it to *0.97 is ALMOST SIMILAR, but once you move with the minimap, the position of the server calculation is always faster than the client
//						but once only using the real map around your ship, its almost too similar
///////////////////////////////////////////
class CMovementManager {
	typedef long long duration_t;
	
	Position_t	local_pos;
	Position_t	planned_pos;
	std::atomic<speed_t>		speed;
	std::atomic<duration_t>	time_last_call;
	std::atomic<duration_t>	time_for_destination;

	//for move and time getter
	double		tmpz;
	int			delta_x;
	int			delta_y;

public:
	CMovementManager(Position_t local_pos, speed_t speed) : local_pos(local_pos), planned_pos(local_pos),
		speed(static_cast<speed_t>(speed*0.97)), time_last_call(get_current_time()), time_for_destination(0),
		tmpz(0),delta_x(0),delta_y(0)
	{ }
	CMovementManager(pos_t local_pos_x, pos_t local_pos_y,speed_t speed) : CMovementManager(std::make_pair(local_pos_x, local_pos_y),speed) { }
	CMovementManager() : CMovementManager(std::make_pair(0, 0), 1) {}
	void		move(Position_t& planned);
	void		move(pos_t x, pos_t y);

	duration_t	get_time_for_destination();
	duration_t	recalc_time_for_destination();

	Position_t	get_current_position() const;
	pos_t		get_current_position_x() const;
	pos_t		get_current_position_y() const;
	Position_t  get_planned_position() const;

	void		set_speed(speed_t speed);
	void		reset(Position_t& now_position);
	void		reset(pos_t x, pos_t y);
private:

	const duration_t get_current_time() const { 
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

};
#endif