#include "stdafx.h"
#include "file.h"

#include <iostream>

namespace Writer
{

using namespace std;

File::File(string &target)
{
	file.open(target, fstream::out | fstream::trunc);

	desc = "file (" + target + ")";
}

bool File::ready()
{
	return file.is_open();
}

void File::write(int64_t duration, unsigned int step, unsigned int total)
{
	if (step > 0)
	{
		if (step % 50 == 0)
		{
			cout << "  " << (step*100/total) << "%" << endl;
		}
		else if (step % 10 == 0)
		{
			cout << " ";
		}
	}

	cout << ".";

	if (step == total)
	{
		cout << endl;
	}

	file << duration << endl;
}

File::~File()
{
	file.close();
}

}