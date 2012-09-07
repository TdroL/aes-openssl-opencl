#include "stdafx.h"
#include "file.h"
#include <iomanip>

#include <iostream>
#include "boost/filesystem.hpp"

namespace Writer
{

using namespace std;
namespace fs = boost::filesystem;

File::File(string &target)
{
	fs::path parent_path = fs::path(target).parent_path();

	if ( ! parent_path.empty() && ! fs::is_directory(parent_path))
	{
		fs::create_directory(parent_path);
	}

	file.open(target, fstream::out | fstream::trunc);

	desc = "file (" + target + ")";
}

bool File::ready()
{
	return file.is_open();
}

void File::write(string &duration, unsigned int step, unsigned int total)
{
	cout << ".";

	step++;
	if (step % 50 == 0 || step == total)
	{
		cout << "  " << (step * 100 / total) << "%" << endl;
	}
	else if (step % 10 == 0)
	{
		cout << " ";
	}

	file << duration << endl;
}

File::~File()
{
	file.close();
}

}