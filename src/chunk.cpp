#include "chunk.h"
#include "memory.h" // For GROW_ARRAY, FREE_ARRAY macros
#include "value.h"  // For Value and ValueArray operations

/**
 * Initializes a new empty bytecode chunk.
 *
 * @param chunk Pointer to uninitialized Chunk structure
 *
 * @note Sets all fields to zero/NULL state
 * @note Initializes the constant pool as empty
 */
void initChunk(Chunk* chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

/**
 * Appends a byte to the chunk's bytecode array with line number information.
 *
 * @param chunk Target chunk to modify
 * @param byte The bytecode instruction or operand to append
 * @param line Source line number for debugging information
 *
 * @note Automatically grows the arrays if capacity is insufficient
 * @note Maintains parallel code/lines arrays for runtime debugging
 */
void writeChunk(Chunk* chunk, uint8_t byte, int line)
{
    // Check if we need to grow the arrays
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);

        // Grow the bytecode array
        chunk->code = GROW_ARRAY(
            uint8_t,
            chunk->code,
            oldCapacity,
            chunk->capacity);

        // Grow the line information array in parallel
        chunk->lines = GROW_ARRAY(
            int,
            chunk->lines,
            oldCapacity,
            chunk->capacity);
    }

    // Store the byte and its line information
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

/**
 * Releases all memory used by a chunk and resets it to empty state.
 *
 * @param chunk Chunk to deallocate
 *
 * @note Frees both bytecode and line number arrays
 * @note Also frees the constant pool
 * @note Leaves the chunk in valid empty state (can be reused)
 */
void freeChunk(Chunk* chunk)
{
    // Free the bytecode storage
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);

    // Free the line information storage
    FREE_ARRAY(int, chunk->lines, chunk->capacity);

    // Free the constant pool
    freeValueArray(&chunk->constants);

    // Reset to initial empty state
    initChunk(chunk);
}

/**
 * Adds a constant value to the chunk's constant pool.
 *
 * @param chunk Target chunk
 * @param value The value to add to the constant pool
 * @return Index of the newly added constant
 *
 * @note Constants are stored in a ValueArray within the chunk
 * @note Returned index is used by OP_CONSTANT instructions
 */
int addConstant(Chunk* chunk, Value value)
{
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1; // Return new constant's index
}