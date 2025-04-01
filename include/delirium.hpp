#ifndef DELIRIUM_HPP
#define DELIRIUM_HPP

#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace Delirium {
// Exit codes - remain in header as constexpr
namespace ExitCodes {
constexpr int SUCCESS = 0;
constexpr int USAGE_ERROR = 64;
constexpr int COMPILE_ERROR = 65;
constexpr int RUNTIME_ERROR = 70;
constexpr int IO_ERROR = 74;
}

// Forward declarations
[[noreturn]] void throwRuntimeError(std::string const& msg, int code);

// VM lifecycle RAII wrapper
class VMWrapper {
public:
    VMWrapper();
    ~VMWrapper();
    VMWrapper(VMWrapper const&) = delete;
    VMWrapper& operator=(VMWrapper const&) = delete;
};

// File operations
std::string readFile(std::filesystem::path const& path);
void runFile(std::filesystem::path const& path);
}

#endif // DELIRIUM_HPP