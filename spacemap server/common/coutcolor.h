#ifndef __COUTCOLOR_H__
#define __COUTCOLOR_H__
#include "pch.h"
#include <iostream>
#include "Color.h"


std::ostream& operator<<(std::ostream& os, const EColor& c);
template<class El, class Tr>
std::basic_ostream<El, Tr> cendl(std::basic_ostream<El, Tr>&);

// I/O
using std::cout;
using std::cerr;
using std::clog;
#define dcout cout << EColor::LIGHT_BLUE << "[PID: " << std::this_thread::get_id() << "] "
#define warncout cout << EColor::YELLOW
#define successcout cout << EColor::DARK_GREEN
#define kickcout cout << EColor::DARK_BLUE
#define errorcout cout << EColor::RED
#define BASIC_DEBUG_FUNCTION_COUT cout << EColor::PURPLE << __FUNCTION__ << " CALL!" << cendl;

#include "coutcolor.tpp"

#endif //__COLOR_H__