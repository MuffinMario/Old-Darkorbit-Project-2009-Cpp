#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

#include <boost/date_time.hpp>

#include "FileReader.h"
class CFileWriterException : public std::exception {
public:
	CFileWriterException(const char* exc) : std::exception(exc) {}
};
class CFileWriter {
	//extremely simple file writer for simple debug/hacking attempt checks
	std::mutex		writeMutex;
	std::string		path;
	std::string		textbuffer;
	std::fstream	savefile;
public:
	explicit CFileWriter(const std::string& p) : path(p) { 
		savefile.open(path.c_str(), std::ios::app); 
		if (savefile.is_open() == false) {
			std::stringstream ss;
			ss << "Couldn't open file " << p;
			throw CFileWriterException(ss.str().c_str());
		}
		writemore("Starting File Logging System!", "Start");
	
	}
	void writemore(const std::string& str2,std::string bracketText);
};
#endif