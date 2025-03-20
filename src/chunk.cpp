#include "chunk.h"
#include "memory.h"
#include "value.h"

// Initialize the chunk with default values
void initChunk(Chunk* chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

// Write to the chunk
void writeChunk(Chunk* chunk, uint8_t byte, int line)
{
    /*
     * If current array does not have capacity for the new byte,
     * then grow the array to make the room for the new byte.
     * Else, we just append the new byte to the chunk array
     * */

    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(
            uint8_t,
            chunk->code,
            oldCapacity,
            chunk->capacity);
        chunk->lines = GROW_ARRAY(
            int,
            chunk->lines,
            oldCapacity,
            chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

// Deallocate the chunk
void freeChunk(Chunk* chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    // initChunk is called to zero out the fields leaving the chunk
    // in a well defined empty state
    initChunk(chunk);
}

// Add a new constant to the constant pool
int addConstant(Chunk* chunk, Value value)
{
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1; // We return the index where the new constant was appended
}
