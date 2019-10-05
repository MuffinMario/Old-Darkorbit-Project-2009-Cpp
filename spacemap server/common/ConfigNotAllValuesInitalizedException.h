#pragma once
#include <exception>
class CConfigNotAllValuesInitalizedException : public std::exception {
public:
	CConfigNotAllValuesInitalizedException(const char* what) : std::exception(what) {}
};