#include "chunk.h"
#include "debug.h"

int main(int argc, char** argv)
{
    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2); // Add constant to the constant pool
    writeChunk(&chunk, OP_CONSTANT, 123);    // Add opcode to the chunk->code
    writeChunk(&chunk, constant, 123);       // Add operand to the chunk->code

    writeChunk(&chunk, OP_RETURN, 123);
    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);

    return 0;
}
