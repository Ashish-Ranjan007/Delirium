#ifndef COMPILER_H
#define COMPILER_H

#include "chunk.h"
#include "object.h"

ObjFunction* compile(char const* source);

#endif
