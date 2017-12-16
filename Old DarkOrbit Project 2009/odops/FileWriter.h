#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <iostream>
#include <fstream>
#include <string>

#include "FileReader.h"

class FileWriter {
	//unhandly debugging log output, trying to prevent using this though
	std::string		path;
	std::string		port;
	std::string		textbuffer;
	std::fstream	savefile;
public:
	FileWriter(std::string p, unsigned short port_) : path(p), textbuffer(""), port(std::to_string(port_)) {}
	void writemore(std::string str2,unsigned short port = 0);
};
#endif