#ifndef WRITER_H
#define WRITER_H

#include "writer/file.h"
#include "writer/console.h"

namespace Writer
{

std::unique_ptr<Base> factory(std::string &target);

}

#endif