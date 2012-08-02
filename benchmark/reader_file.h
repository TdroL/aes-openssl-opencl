#ifndef READER_FILE_H
#define READER_FILE_H

#include "reader.h"
#include <fstream>

class Reader_File : public Reader
{
public:
	std::fstream file;

	Reader_File(std::string target);

	bool ready();
	std::unique_ptr<Bench::Base::data_type> read(size_t length);

	~Reader_File();
};

#endif