#ifndef WRITER_CONSOLE_H
#define WRITER_CONSOLE_H

#include "writer.h"

class Writer_Console : public Writer
{
public:
	Writer_Console();

	bool ready();
	void write(int64_t duration);

	~Writer_Console();
};

#endif