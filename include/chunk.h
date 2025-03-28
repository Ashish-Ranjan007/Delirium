#ifndef chunk_h
#define chunk_h

#include "common.h"
#include "value.h"

// 1-byte instruction set architecture for the Virtual Machine
typedef enum Opcode {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_MODULO,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_RETURN,
} OpCode;

/*
 * The `Chunk` is a container for the low-level instructions that the VM will execute.
 * It handles the dynamic memory management of those instructions.
 * It is the compiled result of the source code, ready for execution.
 * */
typedef struct Chunk {
    int count;            // Number of bytecode instructions that are currently present in the code array.
    int capacity;         // Total allocated size of the code array.
    uint8_t* code;        // Each byte in this dynacmically allocated array represents a single bytecode instruction.
    ValueArray constants; // Stores a pool of constants or the "constant pool".
    int* lines;           // Each index in this array is the line number for the corresponding byte in the bytecode.
} Chunk;

void initChunk(Chunk* chunk);                          // Initialize new chunk
void writeChunk(Chunk* chunk, uint8_t byte, int line); // Append a new byte to the end of the chunk
void freeChunk(Chunk* chunk);                          // Free Chunk of memory
int addConstant(Chunk* chunk, Value value);            //  Add a new constant to the constant pool

#endif
