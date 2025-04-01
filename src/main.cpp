/*
 * Delirium Programming Language - Main entry point and file handling
 *
 * This file contains the main program logic for the Delirium programming language.
 *
 */

#include "bytecode.hpp"
#include "disassembler.hpp"
#include "opcode.hpp"

/*
 * Main program entry point.
 *
 * @param argc Argument count
 * @param argv Argumet vector
 * @return Appropriate exit code based on program execution
 */
int main(int argc, char** argv)
{
    Bytecode bytecode;

    int constant = bytecode.appendConstant(1.2);
    bytecode.appendBytecode(Delirium::OP_CONSTANT, 123);
    bytecode.appendBytecode(constant, 123);

    int const1 = bytecode.appendConstant(1.3);
    bytecode.appendBytecode(Delirium::OP_CONSTANT, 123);
    bytecode.appendBytecode(const1, 123);

    bytecode.appendBytecode(Delirium::OP_RETURN, 123);
    Delirium::disassembleBytecode(bytecode, "test chunk");
}
