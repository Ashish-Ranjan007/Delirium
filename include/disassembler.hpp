#ifndef DISASSEMBLER_HPP
#define DISASSEMBLER_HPP

#include "bytecode.hpp"
#include <string>

namespace Delirium {

void disassembleBytecode(Bytecode const& bytecode, std::string name);
int disassembleInstruction(Bytecode const& bytecode, int offset);

} // Delirium

#endif
