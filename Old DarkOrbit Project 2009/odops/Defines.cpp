#include "Defines.h"


long long getTimeNow()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

std::string to_string(bool b) {
	return b ? "1" : "0";
}

//silly, this already is a string
std::string to_string(std::string str)
{
	return str;
}
 // first|second
std::string to_string(Position_t pos)
{
	return std::string(to_string(pos.first) + "|" + to_string(pos.second));
}
