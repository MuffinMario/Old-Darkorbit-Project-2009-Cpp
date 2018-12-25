
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
class CPacketer
{

	std::deque<std::string> m_savedPacket;

	std::string getString();
public:
	operator std::string() { return getString(); }
	explicit CPacketer(std::string operationHeader);

	template <typename T>
	T			getValueAt(int i);
	std::string getStringAt(int i);
	void		push(const std::string& cmd);
	void		push(const std::deque<std::string>& i);
	CPacketer	pushAndGet(const std::string& cmd);
	CPacketer	pushAndGet(const std::deque<std::string>& i);
	void		cleanPacket();
private: 
};
#endif