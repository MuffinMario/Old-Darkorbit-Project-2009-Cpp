#ifndef __COLOR_H__
#define __COLOR_H__
#include <iostream>
#include <Windows.h>
enum class EColor
{
	BLACK,
	DARKER_BLUE,
	DARK_GREEN,
	DARK_TURQUOISE,
	DARK_RED,
	DARK_PURPLE,
	DARK_YELLOW,
	DEFAULT,
	WHITE = 7,
	GREY,
	DARK_BLUE,
	GREEN,
	LIGHT_BLUE,
	RED,
	PURPLE,
	YELLOW,
	OTHER_WHITE
};
struct _ColorChange
{
	_ColorChange(int color)
	{
		SetConsoleTextAttribute(hConsole, color);
	}
private:
	static HANDLE hConsole;
};
#endif