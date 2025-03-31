#ifndef DEBUG_H
#define DEBUG_H

#include "chunk.h" // For Chunk type definitions

/**
 * Disassembles an entire chunk of bytecode for debugging.
 *
 * @param chunk Pointer to the Chunk containing bytecode to disassemble
 * @param name Descriptive name for the chunk (e.g., "main script")
 *
 * @note Outputs human-readable bytecode to stdout including:
 *       - Instruction mnemonics
 *       - Constant pool indices
 *       - Line number information
 */
void disassembleChunk(Chunk* chunk, char const* name);

/**
 * Disassembles a single instruction at the given offset.
 *
 * @param chunk Pointer to the Chunk containing the instruction
 * @param offset Byte offset within the chunk's code array
 * @return New byte offset after processing the current instruction
 *
 * @note Handles all opcodes defined in OpCode enum
 * @note Returns offset to next instruction for sequential disassembly
 */
int disassembleInstruction(Chunk* chunk, int offset);

#endif // DEBUG_H