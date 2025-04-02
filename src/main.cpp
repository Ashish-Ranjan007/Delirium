// main.cpp - Main entry point for the Delirium language interpreter
// Bytecode virtual machine implementation for the Delirium programming language

#include <cstdlib>  // For exit() and EXIT_* codes
#include <cstring>  // For string operations
#include <fstream>  // For file I/O
#include <ios>      // For std::ios flags
#include <iostream> // For std::cerr

#include "vm.h" // Delirium Virtual Machine implementation

/**
 * Reads the contents of a Delirium source file into memory.
 *
 * @param path Path to the .del source file
 * @return Dynamically allocated buffer containing the source code
 * @throws Exits with status 74 (EX_IOERR) on file operations failure
 */
static std::string readFile(std::string const& path) // Accept std::string
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "[Delirium] Could not open file: " << path << std::endl;
        exit(74);
    }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (source.empty()) {
        std::cerr << "[Delirium] Invalid file size for: " << path << std::endl;
        exit(74);
    }

    return source;
}

/**
 * Executes a Delirium source file.
 *
 * @param path Path to the .del file to execute
 * @throws Exits with status 65 (EX_DATAERR) for syntax errors,
 *         70 (EX_SOFTWARE) for runtime errors
 */
static void runFile(std::string const& path)
{
    // Check if file has .del extension
    if (path.size() < 4 || path.substr(path.size() - 4) != ".del") {
        std::cerr << "[Delirium] Error: File must have .del extension\n";
        exit(64);
    }

    std::string source = readFile(path);
    std::string modifiablePath = path;
    InterpretResult result = interpret(source.c_str(), modifiablePath);

    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);
}

/**
 * Delirium Language Interpreter
 * ============================
 * Usage:
 *   delirium [script.del]
 *
 * Exit Codes:
 *   0 - Success
 *   64 - Command line usage error
 *   65 - Compilation error (invalid syntax)
 *   70 - Runtime error
 *   74 - I/O error (file operations)
 */
int main(int argc, char** argv)
{
    initVM();

    if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cerr << "Delirium Language Interpreter\nUsage: delirium [script.dlm]\n";
        exit(64);
    }

    freeVM();
    return 0;
}
