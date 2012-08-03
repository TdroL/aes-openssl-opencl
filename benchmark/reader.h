#ifndef READER_H
#define READER_H

#include "reader/file.h"
#include "reader/dummy.h"

namespace Reader
{

std::unique_ptr<Base> factory(std::string &target);

}

#endif