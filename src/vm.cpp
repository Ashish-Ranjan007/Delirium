#include <cstdint>  // For integer types
#include <cstring>  // For string operations
#include <iostream> // For I/O operations
#include <memory.h> // For memory operations
#include <stdarg.h> // For variable arguments
#include <string>   // For string handling
#include <time.h>   // For clock() function

#include "chunk.h"    // For bytecode chunks
#include "common.h"   // For common definitions
#include "compiler.h" // For code compilation
#include "debug.h"    // For debugging utilities
#include "lexer.h"
#include "memory.h" // For memory management
#include "mutator.h"
#include "object.h" // For object system
#include "value.h"  // For value representation
#include "vm.h"     // For VM definitions

// Single global VM instance
VM vm;
std::string sourcePath;

/**
 * Native clock() function exposed to Delirium.
 *
 * @param argCount Number of arguments (must be 0)
 * @param args Argument array (unused)
 * @return Current time in seconds as number
 */
static Value clockNative(int argCount, Value* args)
{
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

/**
 * Resets the VM's call stack to empty state.
 */
static void resetStack()
{
    vm.stackTop = vm.stack; // Reset stack pointer
    vm.frameCount = 0;      // Clear call frames
}

/**
 * Reports a runtime error with stack trace.
 *
 * @param format printf-style format string
 * @param ... Variable arguments for message
 */
static void runtimeError(char const* format, ...)
{
#ifdef DEBUG_MUTATE_CODE
    char const* lex = getLexer();
    Mutator mut = Mutator(lex, sourcePath);
    mut.mutateCode();

    resetStack();
    return;
#endif

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // Print stack trace
    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->function;
        // Calculate instruction offset in chunk
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ",
            function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    resetStack();
}

/**
 * Defines a native function in the global namespace.
 *
 * @param name Name of the native function
 * @param function Pointer to native implementation
 */
static void defineNative(char const* name, NativeFn function)
{
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

/**
 * Initializes the virtual machine to empty state.
 */
void initVM()
{
    resetStack();
    vm.objects = NULL;                  // Empty object list
    initTable(&vm.strings);             // Empty string table
    initTable(&vm.globals);             // Empty global namespace
    defineNative("clock", clockNative); // Built-in clock()
}

/**
 * Releases all resources used by the VM.
 */
void freeVM()
{
    freeTable(&vm.globals); // Free global variables
    freeTable(&vm.strings); // Free interned strings
    freeObjects();          // Free all allocated objects
}

/**
 * Pushes a value onto the VM's stack.
 *
 * @param value Value to push
 */
void push(Value value)
{
    *vm.stackTop = value; // Store value
    vm.stackTop++;        // Move stack pointer
}

/**
 * Pops a value from the VM's stack.
 *
 * @return The popped value
 */
Value pop()
{
    vm.stackTop--;       // Move stack pointer
    return *vm.stackTop; // Return value
}

/**
 * Peeks at a value on the stack without popping it.
 *
 * @param distance Number of slots from top to peek (0 = top)
 * @return The value at that stack position
 */
static Value peek(int distance)
{
    return vm.stackTop[-1 - distance];
}

/**
 * Calls a Delirium function.
 *
 * @param function Function to call
 * @param argCount Number of arguments passed
 * @return true if call succeeded, false on error
 */
static bool call(ObjFunction* function, int argCount)
{
    // Check arity
    if (argCount != function->arity) {
        runtimeError("Expected %d arguments but got %d.",
            function->arity, argCount);
        return false;
    }

    // Check stack depth
    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    // Setup new call frame
    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

/**
 * Calls any callable value (function or native).
 *
 * @param callee Callable value to invoke
 * @param argCount Number of arguments
 * @return true if call succeeded, false on error
 */
static bool callValue(Value callee, int argCount)
{
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
        case OBJ_FUNCTION:
            return call(AS_FUNCTION(callee), argCount);
        case OBJ_NATIVE: {
            NativeFn native = AS_NATIVE(callee);
            Value result = native(argCount, vm.stackTop - argCount);
            vm.stackTop -= argCount + 1;
            push(result);
            return true;
        }
        default:
            break; // Non-callable object type
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}

/**
 * Checks if a value is falsey (nil or false).
 *
 * @param value Value to check
 * @return true if falsey, false otherwise
 */
static bool isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

/**
 * Concatenates two strings from the stack.
 */
static void concatenate()
{
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    push(OBJ_VAL(result));
}

/**
 * Runs the bytecode in the current call frame.
 *
 * @return Interpretation result status
 */
static InterpretResult run()
{
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

// Bytecode reading macros
#define READ_BYTE() (*frame->ip++)

#define READ_SHORT() \
    (frame->ip += 2, \
        (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() \
    (frame->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(valueType, op)                          \
    do {                                                  \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be numbers.");    \
            return INTERPRET_RUNTIME_ERROR;               \
        }                                                 \
        double b = AS_NUMBER(pop());                      \
        double a = AS_NUMBER(pop());                      \
        push(valueType(a op b));                          \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << "        " << std::endl;
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            std::cout << "[ ";
            printValue(*slot);
            std::cout << " ]";
        }
        std::cout << std::endl;
        disassembleInstruction(&frame->function->chunk,
            (int)(frame->ip - frame->function->chunk.code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
        case OP_CONSTANT: {
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        }
        case OP_NIL:
            push(NIL_VAL);
            break;
        case OP_TRUE:
            push(BOOL_VAL(true));
            break;
        case OP_FALSE:
            push(BOOL_VAL(false));
            break;
        case OP_POP:
            pop();
            break;
        case OP_SET_LOCAL: {
            uint8_t slot = READ_BYTE();
            frame->slots[slot] = peek(0);
            break;
        }
        case OP_GET_LOCAL: {
            uint8_t slot = READ_BYTE();
            push(frame->slots[slot]);
            break;
        }
        case OP_GET_GLOBAL: {
            ObjString* name = READ_STRING();
            Value value;
            if (!tableGet(&vm.globals, name, &value)) {
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            push(value);
            break;
        }
        case OP_DEFINE_GLOBAL: {
            ObjString* name = READ_STRING();
            tableSet(&vm.globals, name, peek(0));
            pop();
            break;
        }
        case OP_SET_GLOBAL: {
            ObjString* name = READ_STRING();
            if (tableSet(&vm.globals, name, peek(0))) {
                tableDelete(&vm.globals, name);
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_EQUAL: {
            Value b = pop();
            Value a = pop();
            push(BOOL_VAL(valuesEqual(a, b)));
            break;
        }
        case OP_GREATER:
            BINARY_OP(BOOL_VAL, >);
            break;
        case OP_LESS:
            BINARY_OP(BOOL_VAL, <);
            break;
        case OP_ADD: {
            if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                concatenate();
            } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                // Approach 1
                // double b = AS_NUMBER(pop());
                // double a = AS_NUMBER(pop());
                // push(NUMBER_VAL(a + b));

                // Approach 2
                BINARY_OP(NUMBER_VAL, +);
            } else {
                runtimeError("Operands must be two numbers or two strings.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_SUBTRACT:
            BINARY_OP(NUMBER_VAL, -);
            break;
        case OP_MULTIPLY:
            BINARY_OP(NUMBER_VAL, *);
            break;
        case OP_DIVIDE:
            BINARY_OP(NUMBER_VAL, /);
            break;
        case OP_NOT:
            push(BOOL_VAL(isFalsey(pop())));
            break;
        case OP_NEGATE:
            if (!IS_NUMBER(peek(0))) {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(NUMBER_VAL(-AS_NUMBER(pop())));
            break;
        case OP_PRINTLN:
            printValue(pop());
            std::cout << std::endl;
            break;
        case OP_PRINT:
            printValue(pop());
            break;
        case OP_JUMP: {
            uint16_t offset = READ_SHORT();
            frame->ip += offset;
            break;
        }
        case OP_JUMP_IF_FALSE: {
            uint16_t offset = READ_SHORT();
            if (isFalsey(peek(0)))
                frame->ip += offset;
            break;
        }
        case OP_LOOP: {
            uint16_t offset = READ_SHORT();
            frame->ip -= offset;
            break;
        }
        case OP_CALL: {
            int argCount = READ_BYTE();
            if (!callValue(peek(argCount), argCount)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        case OP_RETURN: {
            Value result = pop();
            vm.frameCount--;
            if (vm.frameCount == 0) {
                pop();
                return INTERPRET_OK;
            }

            vm.stackTop = frame->slots;
            push(result);
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

/**
 * Interprets Delirium source code.
 *
 * @param source Source code to execute
 * @return Interpretation result status
 */
InterpretResult interpret(char const* source, std::string& path)
{
    sourcePath = path;
    ObjFunction* function = compile(source);
    if (function == NULL)
        return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    call(function, 0);

    return run();
}
