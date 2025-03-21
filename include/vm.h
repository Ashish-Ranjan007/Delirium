#ifndef VM_H
#define VM_H

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct VM {
    Chunk* chunk;
    uint8_t* ip; // Instruction Pointer or Program Counter
    Value stack[STACK_MAX];
    Value* stackTop;
} VM;

typedef enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();                                 // Initialize a virtual machine
void freeVM();                                 // Destroy a virtual machine
InterpretResult interpret(char const* source); // Main entry point into the VM
void push(Value value);
Value pop();

#endif
