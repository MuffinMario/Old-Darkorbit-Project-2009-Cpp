#include <iostream>
#include "Color.h" // lastcolor
extern EColor _lastColor;
template<class El, class Tr>
std::basic_ostream<El, Tr>& cendl(std::basic_ostream<El, Tr>& os)
{
	if(_lastColor != EColor::DEFAULT)
	{
		os << EColor::DEFAULT;
	}
	return os << std::endl;
}