#ifndef __COLOR_H__
#define __COLOR_H__
#include <iostream>
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
	explicit _ColorChange(int color);
private:
	static void* hConsole; // HANDLE is void* but to prevent windows.h in this header we use void*
};
#endif