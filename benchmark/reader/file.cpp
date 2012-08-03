#include "stdafx.h"
#include "file.h"

namespace Reader
{

using namespace std;

File::File(string target)
{
	file.open(target);

	desc = "file: " + target;
}

bool File::ready()
{
	return file.is_open();
}

unique_ptr<Bench::Base::data_type> File::read(size_t length)
{
	unique_ptr<Bench::Base::data_type> data(new Bench::Base::data_type[length]);

	return data;
}

File::~File()
{
	file.close();
}

}
