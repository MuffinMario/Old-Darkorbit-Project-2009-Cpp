#include "FileWriter.h"
extern FileWriter filewrite;
void FileWriter::writemore(std::string str2, unsigned short whichport)
{
	if (whichport != 0)
		port = "[" + std::to_string(whichport) + "] ";
	textbuffer = port + str2;
	savefile.open("C:/xampp/htdocs/log.txt", std::ofstream::app);
	
	savefile << textbuffer << std::endl;
	savefile.close();
}
