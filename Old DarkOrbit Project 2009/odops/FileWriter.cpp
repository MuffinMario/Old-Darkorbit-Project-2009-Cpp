#include "FileWriter.h"
void CFileWriter::writemore(std::string str2, std::string brackettext)
{
	static std::mutex write_m;
	std::string addition = "";
	if (brackettext != "")
		if(brackettext.size() <= 5)
			addition = "[" + brackettext + "]\t\t";
		else
			addition = "[" + brackettext + "]\t";
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	auto d = now.date();
	std::string _m = boost::lexical_cast<std::string>(d.month().as_number());
	std::string _d = boost::lexical_cast<std::string>(d.day().as_number());
	//just in ANY case theres going to be an interference (which i doubt will EVER happen, but in case it does: well)
	write_m.lock();
	savefile << d.year() << "-"
		<< (_m.size() == 1 ? "0" + _m : _m) << "-"
		<< (_d.size() == 1 ? "0" + _d : _d) << " "
		<< now.time_of_day() << " \t" 
		<< addition 
		<< str2 
		<< std::endl;
	write_m.unlock();
}
