#include <cstdint>
#include <iostream>
#include <vector>

#include "bytecode.hpp"
#include "disassembler.hpp"
#include "opcode.hpp"

/*===========================Static Functions===========================*/
static int simpleInstruction(std::string name, int offset)
{
    std::cout << name << std::endl;
    return offset + 1;
}

/*==========================Delirum Namespace===========================*/
int Delirium::disassembleInstruction(Bytecode const& bytecode, int offset)
{
    printf("%04d ", offset);
    std::vector<uint8_t> const& code = bytecode.getCode();

    if (offset >= static_cast<int>(code.size())) {
        std::cout << "Error: Offset out of bounds" << std::endl;
        return offset + 1; // Avoid further errors
    }

    uint8_t instruction = code[offset];
    switch (instruction) {
    case Delirium::OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    default:
        std::cout << "Unknown opcode " << static_cast<int>(instruction) << std::endl;
        return offset + 1;
    }
}

void Delirium::disassembleBytecode(Bytecode const& bytecode, std::string name)
{
    std::cout << "===== " << name << " =====" << std::endl;
    std::vector<uint8_t> const& code = bytecode.getCode();

    for (size_t offset = 0; offset < code.size();) {
        offset = disassembleInstruction(bytecode, static_cast<int>(offset));
    }
}
