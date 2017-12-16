#include "Packeter.h"

template<typename T>
T Packeter::getValueAt(int i)
{
	try {
		//convert the packet at index i from "50|" to "50" to T 50
		return boost::lexical_cast<T>(getStringAt(i));
	}
	catch(std::out_of_range oof) { std::cerr << __FILE__ << "|" << __LINE__ << "|Out of range in function " << __FUNCTION__ << " requesting index " << i  << " of " << savedPacket.size() - 1 << std::endl; }
	catch (boost::bad_lexical_cast& e) { std::cerr << __FILE__ << "|" << __LINE__ << "|Bad lexical cast in function " << __FUNCTION__ << " requesting index " << i << " of " << savedPacket.size() - 1 << std::endl; }
}

std::string Packeter::getStringAt(int i) {
	std::string t = savedPacket.at(i);
	boost::erase_all(t, "|");
	return t;
}
Packeter::Packeter(std::string operationHeader) {
	if (!boost::algorithm::starts_with(operationHeader, "ERR")) {
		savedPacket.emplace_back("0");
		push(operationHeader);
	}
	else {
		savedPacket.emplace_back(operationHeader);
	}
}

std::string Packeter::getString()
{
	std::string tmp = "";
	for (auto i = savedPacket.begin(); i != savedPacket.end(); i++) {
		tmp += *i;
	}
	return tmp;
}

void Packeter::push(std::string cmd)
{
	savedPacket.push_back("|" + cmd );
}

void Packeter::push(std::deque<std::string> i)
{
	for (auto& it : i) {
		push(it);
	}
}

Packeter Packeter::pushAndGet(std::string cmd)
{
	push(cmd);
	return *this;
}

Packeter Packeter::pushAndGet(std::deque<std::string> i)
{
	push(i);
	return *this;
}

void Packeter::cleanPacket()
{
	savedPacket.clear();
}
