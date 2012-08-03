#include "stdafx.h"
#include "file.h"

namespace Writer
{

using namespace std;

File::File(string &target)
{
	file.open(target);

	desc = "file: " + target;
}

bool File::ready()
{
	return file.is_open();
}

void File::write(int64_t duration)
{

}

File::~File()
{
	file.close();
}

}