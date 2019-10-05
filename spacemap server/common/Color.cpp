#include "pch.h"
#include "Color.h"
#include <Windows.h> // winsock issues if included first in header
HANDLE _ColorChange::hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 

_ColorChange::_ColorChange(int color)
{
	SetConsoleTextAttribute(hConsole, color);
}
