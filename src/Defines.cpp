#include "Defines.h"
#include <iomanip>

std::ostream& operator<<(std::ostream& os, Position_t pos)
{
	return os << "(" << pos.first << "|" << pos.second << ")";
}
long long getTimeNow()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}
long long getTimeNowDelta(long long offset)
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() + offset;
}

bool timeHasPassed(long long timestamp, long long offset) {
	return timestamp + offset < getTimeNow();
}
bool timeHasPassed(long long timestamp) {
	return timestamp < getTimeNow();
}
std::string to_string(bool b) {
	return b ? "1" : "0";
}

std::string to_string(float t) {
	// no lexical cast :(
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << t;
	return ss.str();
}

std::string to_string(double t) {
	// no lexical cast :(
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << t;
	return ss.str();
}

//silly, this already is a string
std::string to_string(std::string& str)
{
	return str;
}
 // first|second
std::string to_string(Position_t pos)
{
	return std::string(to_string(pos.first) + "|" + to_string(pos.second));
}
