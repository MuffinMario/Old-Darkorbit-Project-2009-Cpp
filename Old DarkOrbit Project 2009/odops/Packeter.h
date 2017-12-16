
#ifndef PACKETER_H
#define PACKETER_H
#include <iostream>
#include <string>
#include <deque>
#include <boost\algorithm\string.hpp>
#include <boost\lexical_cast.hpp>
//////////////////////////////////////////////////////////
//		Packet Parsing
//		This class provides a util-like system which automatically adds | between packet lines so you dont need to do it
//		send/write - only
//////////////////////////////////////////////////////////
class Packeter
{

	std::deque<std::string> savedPacket;

	std::string getString();
public:
	operator std::string() { return getString(); }
	Packeter(std::string operationHeader);

	template <typename T>
	T			getValueAt(int i);
	std::string getStringAt(int i);
	void		push(std::string cmd);
	void		push(std::deque<std::string> i);
	Packeter	pushAndGet(std::string cmd);
	Packeter	pushAndGet(std::deque<std::string> i);
	void		cleanPacket();
private: 
};
#endif