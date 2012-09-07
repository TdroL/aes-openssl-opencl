#include "stdafx.h"
#include "file.h"

namespace Reader
{

using namespace std;

File::File(string target)
{
	file.open(target, ios::in | ios::out);

	desc = "file \"" + target + "\"";
}

bool File::ready()
{
	return file.is_open();
}

unique_ptr<Bench::Container> File::read(size_t length)
{
	file.seekg (0, ios::end);
	streampos fileLength = file.tellg();
	file.seekg (0, ios::beg);

	Bench::Container *sample = new Bench::Container(static_cast<size_t>(fileLength));
	assert(sample->data != nullptr);

	file.read(reinterpret_cast<char *>(sample->data), fileLength);

	return unique_ptr<Bench::Container>(sample);
}

bool File::write(Bench::Container &sample)
{
	assert(sample.data != nullptr);

	file.seekp(0, ios::beg);
	file.write(reinterpret_cast<char *>(sample.data), sample.length * sizeof(Bench::Container::data_type)).flush();

	return true;
}

File::~File()
{
	file.close();
}

}
