#include <iostream>

#include "mutator.h"

Mutator::Mutator(char const* source)
{
    this->source = source;
}

void Mutator::printSource()
{
    std::cout << source << std::endl;
}
