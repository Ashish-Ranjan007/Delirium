#include <cstdint>
#include <iostream>
#include <vector>

#include "bytecode.hpp"
#include "disassembler.hpp"
#include "opcode.hpp"
#include "type.hpp"

/*===========================Static Functions===========================*/
static int simpleInstruction(std::string name, int offset)
{
    std::cout << name << std::endl;
    return offset + 1;
}

static int constantInstruction(std::string name, Bytecode const& bytecode, int offset)
{
    std::vector<uint8_t> const& code = bytecode.getBytecode();
    std::vector<Delirium::Value> const& constants = bytecode.getConstant();

    uint8_t constantIndex = code[offset + 1];
    printf("%-16s %4d '", name.c_str(), constantIndex);
    Delirium::printValue(constants[constantIndex]);
    std::cout << std::endl;

    return offset + 2;
}

/*==========================Delirum Namespace===========================*/
int Delirium::disassembleInstruction(Bytecode const& bytecode, int offset)
{
    printf("%04d ", offset);
    std::vector<uint8_t> const& code = bytecode.getBytecode();

    if (offset >= static_cast<int>(code.size())) {
        std::cout << "Error: Offset out of bounds" << std::endl;
        return offset + 1; // Avoid further errors
    }

    uint8_t instruction = code[offset];
    switch (instruction) {
    case Delirium::OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    case Delirium::OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", bytecode, offset);
    default:
        std::cout << "Unknown opcode " << static_cast<int>(instruction) << std::endl;
        return offset + 1;
    }
}

void Delirium::disassembleBytecode(Bytecode const& bytecode, std::string name)
{
    std::cout << "===== " << name << " =====" << std::endl;
    std::vector<uint8_t> const& code = bytecode.getBytecode();

    for (size_t offset = 0; offset < code.size();) {
        offset = disassembleInstruction(bytecode, static_cast<int>(offset));
    }
}
