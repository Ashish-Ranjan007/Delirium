#include <cstdint>
#include <cstdlib>
#include <vector>

#include "bytecode.hpp"
#include "type.hpp"

Bytecode::Bytecode()
{
    // In this case, the default constructor of std::vector is likely sufficient.
}

Bytecode::~Bytecode()
{
    // Cleanup code for the Bytecode object
}

void Bytecode::appendBytecode(uint8_t byte, int line)
{
    code.push_back(byte);
    lines.push_back(line);
}

int Bytecode::appendConstant(Delirium::Value value)
{
    constants.push_back(value);
    return constants.size() - 1;
}

std::vector<int> const& Bytecode::getLine() const
{
    return lines;
}

std::vector<uint8_t> const& Bytecode::getBytecode() const
{
    return code;
}

std::vector<Delirium::Value> const& Bytecode::getConstant() const
{
    return constants;
}
