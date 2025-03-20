#include <iostream>

#include "chunk.h"
#include "debug.h"

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

static int simpleInstruction(char const* name, int offset)
{
    std::cout << name << std::endl;
    return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset);

    uint8_t instruction = chunk->code[offset];

    switch (instruction) {
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    default:
        std::cout << "Unknown opcode " << instruction << std::endl;
        return offset + 1;
    }
}
