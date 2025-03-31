#ifndef VM_H
#define VM_H

#include "chunk.h"  // Bytecode chunk definitions
#include "object.h" // Object system definitions
#include "table.h"  // Hash table implementation
#include "value.h"  // Value type definitions

// ======================
// Virtual Machine Configuration
// ======================

/**
 * Maximum call frame depth for function calls.
 * This limits the call stack depth to prevent stack overflows.
 */
#define FRAMES_MAX 64

/**
 * Maximum stack size calculated as:
 * (max frames) * (maximum local variables per function)
 * UINT8_COUNT is assumed to be 256 (from value.h/chunk.h)
 */
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

// ======================
// Call Frame Structure
// ======================

/**
 * Represents a single function call frame in the VM.
 * Tracks execution state for each function invocation.
 */
typedef struct CallFrame {
    ObjFunction* function; // The function being executed
    uint8_t* ip;           // Instruction pointer (current position in bytecode)
    Value* slots;          // Pointer to the function's stack window
} CallFrame;

// ======================
// Virtual Machine State
// ======================

/**
 * Core Virtual Machine structure containing all execution state.
 */
typedef struct VM {
    CallFrame frames[FRAMES_MAX]; // Call stack
    int frameCount;               // Current call stack depth

    Value stack[STACK_MAX]; // Value stack
    Value* stackTop;        // Top of the value stack

    Table globals; // Global variables
    Table strings; // String interning table

    Obj* objects; // Linked list of all heap-allocated objects
} VM;

// ======================
// Interpretation Results
// ======================

/**
 * Result codes from bytecode interpretation.
 */
typedef enum InterpretResult {
    INTERPRET_OK,            // Execution succeeded
    INTERPRET_COMPILE_ERROR, // Compilation failed (syntax/semantic error)
    INTERPRET_RUNTIME_ERROR  // Runtime error occurred
} InterpretResult;

// ======================
// Global VM Instance
// ======================

/**
 * Single global VM instance.
 * Extern declaration - actual storage is in vm.c.
 */
extern VM vm;

// ======================
// Public API
// ======================

/**
 * Initializes the virtual machine state.
 * Must be called before any interpretation.
 */
void initVM();

/**
 * Cleans up all VM resources and allocated memory.
 * Should be called when the VM is no longer needed.
 */
void freeVM();

/**
 * Main entry point for executing Delirium source code.
 *
 * @param source Null-terminated source code string
 * @return Interpretation result code
 */
InterpretResult interpret(char const* source);

/**
 * Pushes a value onto the VM's stack.
 *
 * @param value The value to push
 */
void push(Value value);

/**
 * Pops a value from the VM's stack.
 *
 * @return The popped value
 */
Value pop();

#endif // VM_H