#include "StdAfx.h"
#include "reader_file.h"

using namespace std;

Reader_File::Reader_File(string target)
{
	file.open(target);

	desc = "file: " + target;
}

bool Reader_File::ready()
{
	return file.is_open();
}

unique_ptr<Bench::Base::data_type> Reader_File::read(size_t length)
{
	unique_ptr<Bench::Base::data_type> data(new Bench::Base::data_type[length]);

	return data;
}

Reader_File::~Reader_File()
{
	file.close();
}
