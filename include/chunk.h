#ifndef chunk_h
#define chunk_h

#include "common.h"

// 1-byte instruction set architecture for the Virtual Machine
typedef enum Opcode {
    OP_RETURN,
} OpCode;

/*
 * The `Chunk` is a container for the low-level instructions that the VM will execute.
 * It handles the dynamic memory management of those instructions.
 * It is the compiled result of the source code, ready for execution.
 * */
typedef struct Chunk {
    int count; // Number of bytecode instructions that are currently present in the code array.
    int capacity; // Total allocated size of the code array.
    uint8_t* code; // Each byte in this dynacmically allocated array represents a single bytecode instruction.
} Chunk;

void initChunk(Chunk* chunk); // Initialize new chunk
void writeChunk(Chunk* chunk, uint8_t byte); // Append a new byte to the end of the chunk
void freeChunk(Chunk* chunk); // Free Chunk of memory

#endif
