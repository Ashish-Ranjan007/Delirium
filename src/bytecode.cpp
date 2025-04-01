#include <cstdint>
#include <cstdlib>
#include <vector>

#include "bytecode.hpp"

Bytecode::Bytecode()
{
    // Initialization code for the Bytecode object, if any.
    // In this case, the default constructor of std::vector is likely sufficient.
}

Bytecode::~Bytecode()
{
    // Cleanup code for the Bytecode object, if needed.
    // std::vector handles its own memory management.
}

void Bytecode::appendCode(uint8_t byte)
{
    code.push_back(byte);
}

std::vector<uint8_t> const& Bytecode::getCode() const
{
    return code;
}
