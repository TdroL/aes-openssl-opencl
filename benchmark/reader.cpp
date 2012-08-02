#include "StdAfx.h"
#include "reader.h"
#include "reader_file.h"
#include "reader_random.h"

using namespace std;

unique_ptr<Reader> Reader::factory(std::string &target)
{
	if ( ! target.empty())
	{
		return unique_ptr<Reader>(new Reader_File(target));
	}

	return unique_ptr<Reader>(new Reader_Random);
}


Reader::~Reader()
{
}
