#include "pch.h"
#include "Packeter.h"

template<typename T>
T CPacketer::getValueAt(int i)
{
	try {
		//convert the packet at index i from "50|" to "50" to T 50
		return boost::lexical_cast<T>(getStringAt(i));
	}
	catch(std::out_of_range& oof) { std::cerr <<"("<< __FILE__ << ":" << __LINE__ << ") Out of range in function " << __FUNCTION__ << " requesting index " << i  << " of " << m_savedPacket.size() - 1 << std::endl; }
	catch (boost::bad_lexical_cast& e) { std::cerr << "(" << __FILE__ << ":" << __LINE__ << ") Bad lexical cast in function " << __FUNCTION__ << " requesting index " << i << " of " << m_savedPacket.size() - 1 << std::endl; }
}

std::string CPacketer::getStringAt(int i) {
	std::string t = m_savedPacket.at(i);
	boost::erase_all(t, "|");
	return t;
}
CPacketer::CPacketer(const std::string & operationHeader) {
	if (!boost::algorithm::starts_with(operationHeader, "ERR")) {
		m_savedPacket.emplace_back("0");
		push(operationHeader);
	}
	else {
		m_savedPacket.emplace_back(operationHeader);
	}
}

std::string CPacketer::getString()
{
	std::string tmp = "";
	for (auto i = m_savedPacket.begin(); i != m_savedPacket.end(); i++) {
		tmp += *i;
	}
	return tmp;
}

void CPacketer::push(const std::string& cmd)
{
	m_savedPacket.push_back("|" + cmd );
}

void CPacketer::push(const std::deque<std::string>& i)
{
	for (auto& it : i) {
		push(it);
	}
}

CPacketer CPacketer::pushAndGet(const std::string& cmd)
{
	push(cmd);
	return *this;
}

CPacketer CPacketer::pushAndGet(const std::deque<std::string>& i)
{
	push(i);
	return *this;
}

void CPacketer::cleanPacket()
{
	m_savedPacket.clear();
}
