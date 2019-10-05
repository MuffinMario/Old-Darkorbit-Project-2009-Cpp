#include "pch.h"
#include "coutcolor.h"
EColor _lastColor = EColor::DEFAULT;
std::ostream& operator<<(std::ostream& os, const EColor& c)
{
	_ColorChange(static_cast<int>(c));
	_lastColor = c;
	return os;
}