#ifndef WRITER_CONSOLE_H
#define WRITER_CONSOLE_H

#include "base.h"

namespace Writer
{

class Console : public Writer::Base
{
public:
	Console();

	bool ready();
	void write(std::string &duration, unsigned int step, unsigned int total);

	~Console();
};

}

#endif