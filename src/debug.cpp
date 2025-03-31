#include "debug.h"  // For disassembly declarations
#include "chunk.h"  // For bytecode chunk structure
#include "value.h"  // For constant value printing
#include <iostream> // For output operations

/**
 * Disassembles an entire chunk of bytecode for debugging purposes.
 *
 * @param chunk Pointer to the Chunk to disassemble
 * @param name Descriptive name for the chunk (e.g., "main function")
 *
 * @note Output format:
 *   == chunk name ==
 *   [offset] [line] OPCODE operands
 * @note Handles all instruction types via disassembleInstruction()
 */
void disassembleChunk(Chunk* chunk, char const* name)
{
    std::cout << "== " << name << " ==" << std::endl;

    // Iterate through all instructions in chunk
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

/**
 * Disassembles a constant-load instruction (OP_CONSTANT and variants).
 *
 * @param name Mnemonic name of the instruction
 * @param chunk Containing chunk
 * @param offset Starting byte offset of instruction
 * @return New offset after processing this instruction
 *
 * @format: "OP_CONSTANT    123 'value'"
 */
static int constantInstruction(char const* name, Chunk* chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1]; // Get constant index
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]); // Print constant value
    printf("'\n");
    return offset + 2; // Advance past opcode + operand
}

/**
 * Disassembles a simple instruction with no operands.
 *
 * @param name Mnemonic name of the instruction
 * @param offset Starting byte offset
 * @return New offset after this instruction
 *
 * @format: "OP_NIL"
 */
static int simpleInstruction(char const* name, int offset)
{
    std::cout << name << std::endl;
    return offset + 1; // Advance past single-byte opcode
}

/**
 * Disassembles a byte-argument instruction (e.g., local variable access).
 *
 * @param name Mnemonic name
 * @param chunk Containing chunk
 * @param offset Starting byte offset
 * @return New offset after instruction
 *
 * @format: "OP_GET_LOCAL    4"
 */
static int byteInstruction(char const* name, Chunk* chunk, int offset)
{
    uint8_t slot = chunk->code[offset + 1]; // Get 1-byte operand
    printf("%-16s %4d\n", name, slot);
    return offset + 2; // Advance past opcode + 1-byte operand
}

/**
 * Disassembles a jump instruction with 16-bit offset.
 *
 * @param name Mnemonic name
 * @param sign Direction multiplier (1=forward, -1=backward)
 * @param chunk Containing chunk
 * @param offset Starting byte offset
 * @return New offset after instruction
 *
 * @format: "OP_JUMP        43 -> 87"
 */
static int jumpInstruction(char const* name, int sign,
    Chunk* chunk, int offset)
{
    // Read 16-bit jump offset (big-endian)
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    // Show absolute jump target
    printf("%-16s %4d -> %d\n", name, offset,
        offset + 3 + sign * jump);
    return offset + 3; // Advance past opcode + 2-byte operand
}

/**
 * Disassembles a single instruction at given offset.
 *
 * @param chunk Containing chunk
 * @param offset Byte offset within chunk
 * @return New offset after this instruction
 *
 * @note Output format:
 *   [offset] [line] OPCODE [operands]
 * @note Shows pipe (|) when line number same as previous
 */
int disassembleInstruction(Chunk* chunk, int offset)
{
    // Print instruction offset (4-digit padded)
    printf("%04d ", offset);

    // Show line number or continuation marker
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        std::cout << "    | ";
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    // Decode and print instruction
    uint8_t instruction = chunk->code[offset];

    switch (instruction) {
    case OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_NIL:
        return simpleInstruction("OP_NIL", offset);
    case OP_TRUE:
        return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
        return simpleInstruction("OP_FALSE", offset);
    case OP_POP:
        return simpleInstruction("OP_POP", offset);
    case OP_GET_LOCAL:
        return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
        return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case OP_GET_GLOBAL:
        return constantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OP_DEFINE_GLOBAL:
        return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
        return constantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OP_EQUAL:
        return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
        return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
        return simpleInstruction("OP_LESS", offset);
    case OP_ADD:
        return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return simpleInstruction("OP_DIVIDE", offset);
    case OP_NOT:
        return simpleInstruction("OP_NOT", offset);
    case OP_NEGATE:
        return simpleInstruction("OP_NEGATE", offset);
    case OP_PRINT:
        return simpleInstruction("OP_PRINT", offset);
    case OP_JUMP:
        return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
        return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_LOOP:
        return jumpInstruction("OP_LOOP", -1, chunk, offset);
    case OP_CALL:
        return byteInstruction("OP_CALL", chunk, offset);
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    default:
        std::cout << "Unknown opcode " << instruction << std::endl;
        return offset + 1;
    }
}
