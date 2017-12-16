#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

#include <boost/date_time.hpp>

#include "FileReader.h"

class CFileWriter {
	//extremely simple file writer for simple debug/hacking attempt checks
	std::string		path;
	std::string		textbuffer;
	std::fstream	savefile;
public:
	CFileWriter(std::string p) : path(p) { 
		savefile.open(path.c_str(), std::ios::app); 
		writemore("Starting File Logging System!", "Start");
	
	}
	void writemore(std::string str2,std::string bracketText);
};
#endif