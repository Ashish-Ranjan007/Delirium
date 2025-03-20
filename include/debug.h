#ifndef DEBUG_H
#define DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk* chunk, char const* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif // !DEBUG_H
