#ifndef BYTECODE_HPP
#define BYTECODE_HPP

#include <cstdint>
#include <vector>

class Bytecode {

private:
    std::vector<uint8_t> code;

public:
    Bytecode();
    ~Bytecode();
    void appendCode(uint8_t byte);
    std::vector<uint8_t> const& getCode() const;
};

#endif
