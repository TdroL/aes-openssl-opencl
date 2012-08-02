#ifndef WRITER_FILE_H
#define WRITER_FILE_H

#include "writer.h"
#include <fstream>

class Writer_File : public Writer
{
public:
	std::fstream file;

	Writer_File(std::string &target);

	bool ready();
	void write(int64_t duration);

	~Writer_File();
};

#endif