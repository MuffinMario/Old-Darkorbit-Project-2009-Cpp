#include "MovementManager.h"

void CMovementManager::move(Position_t& planned)
{
	pos_t	tmpz;
	int		delta_x;
	int		delta_y;

	local_pos	= get_current_position();
	planned_pos = planned;

	delta_x = std::abs(planned_pos.first - local_pos.first);
	delta_y = std::abs(planned_pos.second - local_pos.second);

	tmpz = std::sqrt((delta_x * delta_x) + (delta_y * delta_y));

	time_last_call		 = get_current_time();
	time_for_destination = static_cast<long long>(
								static_cast<double>(tmpz) / static_cast<double>(speed) * 1000.0); //distance u / speed u/ms
}

void CMovementManager::move(pos_t x, pos_t y)
{
	pos_t		tmpz;
	int			delta_x;
	int			delta_y;

	local_pos	= get_current_position();
	planned_pos = std::make_pair(x, y);

	delta_x = planned_pos.first  - local_pos.first;
	delta_y = planned_pos.second - local_pos.second;

	tmpz	= std::sqrt((delta_x * delta_x) + (delta_y * delta_y)); // c² = a² + b² amirite friends

	time_last_call		 = get_current_time();
	time_for_destination = static_cast<long long>(
								static_cast<double>(tmpz) / static_cast<double>(speed) * 1000.0);// (time_last_call + tmpz * (speed / 1000)) - time_last_call;

	/*std::cout << "=============" << std::endl
		<< "tmpz: " << tmpz << std::endl
		<< "time_for_destination: " << time_for_destination << std::endl
		<< "time it takes when i calculate it this way: " << tmpz * speed * 1000 << std::endl
		<< "=============" << std::endl;*/
}

CMovementManager::duration_t CMovementManager::get_time_for_destination()
{
	return time_for_destination;
}

Position_t CMovementManager::get_current_position() const
{
	duration_t now_call = get_current_time();
	duration_t timedistance = now_call - time_last_call;

	if (timedistance > time_for_destination) {
		return planned_pos;
	}

	double deg = 0.0;
	pos_t tmpx, tmpy, tmpz;
	double delta_x, delta_y;

	delta_x = planned_pos.first - local_pos.first;
	delta_y = planned_pos.second - local_pos.second;

	//hypothenuse
	tmpz = (static_cast<double>(timedistance) / 1000) * speed;

	//adjacent
	deg = std::atan2(delta_y, delta_x);
	tmpx = std::cos(deg) * tmpz + local_pos.first;

	//opposite
	tmpy = std::sin(deg) * tmpz + local_pos.second;


	/*std::cout << "============" << std::endl;
	std::cout << "now_call: " << now_call << std::endl;
	std::cout << "timedistance: " << timedistance << std::endl;
	std::cout << "deg: " << deg << std::endl;
	std::cout << "tmpx: " << tmpx << std::endl;
	std::cout << "tmpy: " << tmpy << std::endl;
	std::cout << "tmpz: " << tmpz << std::endl;
	std::cout << "delta_x: " << delta_x << std::endl;
	std::cout << "delta_y: " << delta_y << std::endl;
	std::cout << "time_last_call: " << time_last_call << std::endl;
	std::cout << "time_for_destination: " << time_for_destination << std::endl;
	std::cout << "============" << std::endl;*/

	if (local_pos.first <= planned_pos.first && tmpx > planned_pos.first ||
		local_pos.first >= planned_pos.first && tmpx < planned_pos.first) {
		//otherwise would continue moving after reaching goal
		tmpx = planned_pos.first;
	}
	if (local_pos.second <= planned_pos.second && tmpy > planned_pos.second ||
		local_pos.second >= planned_pos.second && tmpy < planned_pos.second) {
		//otherwise would continue moving after reaching goal
		tmpy = planned_pos.second;
	}
	return Position_t(tmpx, tmpy);
}

pos_t CMovementManager::get_current_position_x() const
{
	duration_t now_call		= get_current_time();
	duration_t timedistance = now_call - time_last_call;

	if (timedistance > time_for_destination) {
		return planned_pos.first;
	}

	double	deg = 0.0;
	pos_t	tmpx;
	pos_t	tmpz;
	double	delta_x;
	double	delta_y;

	delta_x = planned_pos.first - local_pos.first;
	delta_y = planned_pos.second - local_pos.second;

	//hypothenuse
	tmpz = (static_cast<double>(timedistance) / 1000) * speed;

	//adjacent
	deg	 = std::atan2(delta_y, delta_x);
	tmpx = std::cos(deg) * tmpz + local_pos.first;

	if (local_pos.first <= planned_pos.first && tmpx > planned_pos.first ||
		local_pos.first >= planned_pos.first && tmpx < planned_pos.first) {
		//otherwise would continue moving after reaching goal
		return planned_pos.first;
	}
	return tmpx;
}

pos_t CMovementManager::get_current_position_y() const
{
	duration_t now_call		 = get_current_time();
	duration_t timedistance	 = now_call - time_last_call;

	if (timedistance > time_for_destination) {
		return planned_pos.second;
	}

	double	deg;
	pos_t	tmpy;
	pos_t	tmpz;
	double	delta_x;
	double	delta_y;

	delta_x = planned_pos.first - local_pos.first;
	delta_y = planned_pos.second - local_pos.second;

	//hypothenuse
	tmpz = (static_cast<double>(timedistance) / 1000) * speed;

	//opposite
	deg  = std::atan2(delta_y, delta_x);
	tmpy = std::sin(deg) * tmpz + local_pos.second;

	if (local_pos.second <= planned_pos.second && tmpy > planned_pos.second ||
		local_pos.second >= planned_pos.second && tmpy < planned_pos.second) {
		//otherwise would continue moving after reaching goal
		return planned_pos.second;
	}
	return tmpy;
}

Position_t CMovementManager::get_planned_position() const
{
	return planned_pos;
}

void CMovementManager::set_speed(speed_t speed)
{
	this->speed = speed*0.97;
}

void CMovementManager::reset(Position_t & now_position)
{
	local_pos	= now_position;
	planned_pos	= local_pos;

	time_last_call		 = get_current_time();
	time_for_destination = 0ll;
}

void CMovementManager::reset(pos_t x, pos_t y)
{
	local_pos	= std::make_pair(x, y);
	planned_pos = local_pos;

	time_last_call		 = get_current_time();
	time_for_destination = 0;
}
