#include "FileReader.h"


std::string CFileReader::getStr(std::string path)
{
	inputstr.open(path);
	while (!inputstr.eof()) {
		getline(inputstr, output);
		//std::cout << output; dbg
	}
	inputstr.close();
	return output;
}
