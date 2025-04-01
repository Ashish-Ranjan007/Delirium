#include "delirium.hpp"
// #include "vm.h" // For VM functions
#include <fstream>
#include <iostream>

namespace Delirium {

// [[noreturn]] implementation
void throwRuntimeError(std::string const& msg, int code)
{
    throw std::runtime_error(msg + " (code: " + std::to_string(code) + ")");
}

// VMWrapper members
VMWrapper::VMWrapper()
{
    // initVM();
}

VMWrapper::~VMWrapper()
{
    // freeVM();
}

// File operations
std::string readFile(std::filesystem::path const& path)
{
    if (!std::filesystem::exists(path)) {
        throwRuntimeError("File not found", ExitCodes::IO_ERROR);
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    auto const size = file.tellg();

    if (size <= 0) {
        throwRuntimeError("Empty file", ExitCodes::IO_ERROR);
    }

    std::string source(size, '\0');
    file.seekg(0);
    file.read(source.data(), size);

    return source;
}

void runFile(std::filesystem::path const& path)
{
    try {
        auto const source = readFile(path);
        std::cout << source << std::endl;
        // switch (interpret(source.c_str())) {
        // case INTERPRET_COMPILE_ERROR:
        //     throwRuntimeError("Compilation failed", ExitCodes::COMPILE_ERROR);
        // case INTERPRET_RUNTIME_ERROR:
        //     throwRuntimeError("Runtime error", ExitCodes::RUNTIME_ERROR);
        // case INTERPRET_OK:
        //     return;
        // }
    } catch (std::exception const& e) {
        std::cerr << "[Delirium] " << e.what() << '\n';
        throw;
    }
}

} // namespace Delirium