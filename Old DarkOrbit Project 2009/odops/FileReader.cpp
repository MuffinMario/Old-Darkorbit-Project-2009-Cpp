#include "FileReader.h"


std::string FileReader::getStr(std::string path)
{
	inputstr.open(path);
	while (!inputstr.eof()) {
		getline(inputstr, output);
		//std::cout << output; dbg
	}
	inputstr.close();
	return output;
}
