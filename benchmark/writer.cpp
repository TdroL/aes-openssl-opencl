#include "stdafx.h"
#include "writer.h"
#include "writer_console.h"
#include "writer_file.h"

using namespace std;

unique_ptr<Writer> Writer::factory(std::string &target)
{
	if ( ! target.empty())
	{
		return unique_ptr<Writer>(new Writer_File(target));
	}

	return unique_ptr<Writer>(new Writer_Console);
}

Writer::~Writer()
{
}
