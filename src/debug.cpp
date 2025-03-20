#include <iostream>

#include "chunk.h"
#include "debug.h"
#include "value.h"

/*
 * All we really do here is loop over the code array field in chunk.
 * And print the index and Opcode for each element (instruction).
 * */

void disassembleChunk(Chunk* chunk, char const* name)
{
    /*
     * Function to print a human-readable representation of the bytecode in Chunk.
     * This function has nothing to do with the working of our project.
     * This is just used as a debug tool for the developer.
     * */

    std::cout << "== " << name << " ==" << std::endl;

    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int constantInstruction(char const* name, Chunk* chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1]; // Get the index of the constant in the constant pool (operand in bytecode)
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]); // Prints the constant at the above determined index in the constant pool
    printf("'\n");

    return offset + 2;
}

static int simpleInstruction(char const* name, int offset)
{
    std::cout << name << std::endl;
    return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset);

    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        std::cout << "    | ";
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];

    switch (instruction) {
    case OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    default:
        std::cout << "Unknown opcode " << instruction << std::endl;
        return offset + 1;
    }
}
