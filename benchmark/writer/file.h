#ifndef WRITER_FILE_H
#define WRITER_FILE_H

#include "base.h"
#include <fstream>

namespace Writer
{

class File : public Writer::Base
{
public:
	std::fstream file;

	File(std::string &target);

	bool ready();
	void write(std::string &duration, unsigned int step, unsigned int total);

	~File();
};

}
#endif