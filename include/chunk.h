#ifndef CHUNK_H
#define CHUNK_H

#include "common.h" // Common definitions and utilities
#include "value.h"  // Value type definitions

// ======================
// Bytecode Instruction Set
// ======================

/**
 * Delirium Virtual Machine Opcodes (1-byte instructions)
 *
 * The VM is a stack-based machine with 1-byte opcodes.
 * Some opcodes (like OP_CONSTANT) use subsequent bytes as operands.
 */
typedef enum OpCode {
    // Constants and literals
    OP_CONSTANT, // Loads constant from constant pool
    OP_NIL,      // Pushes nil value
    OP_TRUE,     // Pushes true value
    OP_FALSE,    // Pushes false value

    // Variable operations
    OP_POP,           // Pops value from stack
    OP_GET_LOCAL,     // Gets local variable by slot index
    OP_SET_LOCAL,     // Sets local variable by slot index
    OP_GET_GLOBAL,    // Gets global variable by name
    OP_DEFINE_GLOBAL, // Defines new global variable
    OP_SET_GLOBAL,    // Sets existing global variable

    // Comparisons
    OP_EQUAL,   // Equality comparison (==)
    OP_GREATER, // Greater-than comparison (>)
    OP_LESS,    // Less-than comparison (<)

    // Arithmetic operations
    OP_ADD,      // Addition (+)
    OP_SUBTRACT, // Subtraction (-)
    OP_MULTIPLY, // Multiplication (*)
    OP_DIVIDE,   // Division (/)
    OP_MODULO,   // Modulo (%)
    OP_NEGATE,   // Unary negation (-)

    // Logical operations
    OP_NOT, // Logical NOT (!)

    // I/O and control flow
    OP_PRINT,         // Prints stack value
    OP_JUMP,          // Unconditional jump
    OP_JUMP_IF_FALSE, // Conditional jump (if false)
    OP_LOOP,          // Jump backward (for loops)
    OP_CALL,          // Calls function
    OP_RETURN,        // Returns from function
} OpCode;

// ======================
// Bytecode Chunk Structure
// ======================

/**
 * Represents a sequence of bytecode instructions and associated data.
 *
 * A chunk is the compiled representation of source code that the VM executes.
 * It uses dynamic arrays to grow as needed during compilation.
 */
typedef struct Chunk {
    int count;            // Current number of bytes in use
    int capacity;         // Total allocated size of code array
    uint8_t* code;        // Dynamic array of bytecode instructions
    ValueArray constants; // Constant pool (literals, strings, etc)
    int* lines;           // Source line numbers for each instruction (debugging)
} Chunk;

// ======================
// Chunk API
// ======================

/**
 * Initializes a new empty chunk.
 *
 * @param chunk Pointer to uninitialized chunk
 */
void initChunk(Chunk* chunk);

/**
 * Appends a byte to the chunk's instruction stream.
 *
 * @param chunk Target chunk
 * @param byte  Instruction byte or operand to append
 * @param line  Source line number for debugging
 */
void writeChunk(Chunk* chunk, uint8_t byte, int line);

/**
 * Releases all memory owned by a chunk.
 *
 * @param chunk Chunk to deallocate
 */
void freeChunk(Chunk* chunk);

/**
 * Adds a constant value to the chunk's constant pool.
 *
 * @param chunk Target chunk
 * @param value Value to add (number, string, etc)
 * @return Index of the constant in the pool
 */
int addConstant(Chunk* chunk, Value value);

#endif // CHUNK_H