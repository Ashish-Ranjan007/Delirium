#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>

#include "chunk.h"
#include "debug.h"
#include "vm.h"

static char* readFile(char const* path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(74);
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    if (fileSize <= 0) {
        std::cerr << "File is empty or could not determine size." << std::endl;
        exit(74);
    }

    file.seekg(0, std::ios::beg);

    char* charBuffer = (char*)malloc(fileSize + 1);
    if (charBuffer == nullptr) {
        std::cerr << "Memory allocation failed." << std::endl;
        exit(74);
    }

    if (!file.read(charBuffer, fileSize)) {
        std::cerr << "Error reading file: " << path << std::endl;
        free(charBuffer);
        exit(74);
    }

    charBuffer[fileSize] = '\0';

    return charBuffer;
}

static void runFile(char const* path)
{
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);
}

int main(int argc, char** argv)
{
    initVM();

    if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cerr << "Usage: clox [path]" << std::endl;
        exit(64);
    }

    freeVM();
    return 0;
}
