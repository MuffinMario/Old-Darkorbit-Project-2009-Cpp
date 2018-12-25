
#ifndef FILEREADER_H
#define FILEREADER_H
#include <string>
#include <iostream>
#include <fstream>

//expandable to your wishes (not that im lazy or something)
class CFileReader
{
	std::ifstream	inputstr;
	std::string		path;
	std::string		output;
public:
	std::string getStr(std::string path);
};
#endif

