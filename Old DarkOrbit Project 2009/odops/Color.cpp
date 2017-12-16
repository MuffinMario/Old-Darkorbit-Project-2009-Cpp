#include "coutcolor.h"

HANDLE _ColorChange::hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
EColor _lastColor = EColor::DEFAULT;
std::ostream& operator<<(std::ostream& os, const EColor& c)
{
	_ColorChange(static_cast<int>(c));
	_lastColor = c;
	return os;
}