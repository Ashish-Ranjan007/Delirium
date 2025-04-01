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

    // Get the constant index from the bytecode array
    uint8_t constantIndex = code[offset + 1];
    printf("%-16s %4d '", name.c_str(), constantIndex);
    // Print the constant from the constant pool using the above index
    Delirium::printValue(constants[constantIndex]);
    std::cout << std::endl;

    return offset + 2;
}

/*==========================Delirum Namespace===========================*/
int Delirium::disassembleInstruction(Bytecode const& bytecode, int offset)
{
    printf("%04d ", offset);
    std::vector<uint8_t> const& code = bytecode.getBytecode();
    std::vector<int> const& lines = bytecode.getLine();

    // Check if offset if out of bounds
    if (offset >= static_cast<int>(code.size())) {
        std::cout << "Error: Offset out of bounds" << std::endl;
        return offset + 1; // Avoid further errors
    }

    // Print line formatting
    if (offset > 0 && lines[offset] == lines[offset - 1]) {
        printf("    | ");
    } else {
        printf("%4d ", lines[offset]);
    }

    // Disassembler logic
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

    // Loop through bytecode disassembling it one instruction at a time
    for (size_t offset = 0; offset < code.size();) {
        offset = disassembleInstruction(bytecode, static_cast<int>(offset));
    }
}
