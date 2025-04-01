#ifndef BYTECODE_HPP
#define BYTECODE_HPP

#include "type.hpp"
#include <cstdint>
#include <vector>

class Bytecode {

private:
    std::vector<uint8_t> code;
    std::vector<Delirium::Value> constants;
    std::vector<int> lines;

public:
    Bytecode();
    ~Bytecode();
    void appendBytecode(uint8_t byte, int line);
    int appendConstant(Delirium::Value value);
    std::vector<int> const& getLine() const;
    std::vector<uint8_t> const& getBytecode() const;
    std::vector<Delirium::Value> const& getConstant() const;
};

#endif
