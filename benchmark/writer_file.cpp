#include "StdAfx.h"
#include "writer_file.h"

using namespace std;

Writer_File::Writer_File(string &target)
{
	file.open(target);

	desc = "file: " + target;
}

bool Writer_File::ready()
{
	return file.is_open();
}

void Writer_File::write(int64_t duration)
{
	
}

Writer_File::~Writer_File()
{
	file.close();
}
