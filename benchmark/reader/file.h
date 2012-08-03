#ifndef READER_FILE_H
#define READER_FILE_H

#include <fstream>
#include "base.h"

namespace Reader
{

class File : public Base
{
public:
	std::fstream file;

	File(std::string target);

	bool ready();
	std::unique_ptr<Bench::Base::data_type> read(size_t length);

	~File();
};

}

#endif