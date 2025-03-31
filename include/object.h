#ifndef OBJECT_H
#define OBJECT_H

#include "chunk.h"  // For Chunk definition
#include "common.h" // For basic types
#include "value.h"  // For Value type

// ======================
// Object Type Predicates
// ======================

/** Gets the ObjType of an object Value */
#define OBJ_TYPE(value) (AS_OBJ(value)->type)

/** Checks if a Value is a function object */
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)

/** Checks if a Value is a native function */
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)

/** Checks if a Value is a string object */
#define IS_STRING(value) isObjType(value, OBJ_STRING)

// ======================
// Object Type Casting
// ======================

/** Safely casts an object Value to ObjFunction* */
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))

/** Extracts the native function pointer from a Value */
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->function)

/** Safely casts an object Value to ObjString* */
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))

/** Gets the C string buffer from a string object Value */
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

// ======================
// Object Type Enum
// ======================

/**
 * Enumeration of all heap-allocated object types in Delirium.
 *
 * The VM uses this to safely downcast Obj pointers to specific types.
 */
typedef enum ObjType {
    OBJ_FUNCTION, // User-defined functions
    OBJ_NATIVE,   // C-implemented native functions
    OBJ_STRING    // String objects (interned)
} ObjType;

// ======================
// Base Object Structure
// ======================

/**
 * Base structure for all heap-allocated Delirium objects.
 *
 * Objects are managed using a linked list for memory management.
 */
struct Obj {
    ObjType type;     // Runtime type tag
    struct Obj* next; // Next object in allocation list
};

// ======================
// Function Object
// ======================

/**
 * Represents a user-defined Delirium function.
 *
 * Contains the function's bytecode and execution context.
 */
typedef struct ObjFunction {
    Obj obj;         // Base object header
    int arity;       // Number of parameters
    Chunk chunk;     // Compiled function body
    ObjString* name; // Function name (or NULL for anonymous)
} ObjFunction;

// ======================
// Native Function
// ======================

/**
 * Function pointer type for C-implemented native functions.
 *
 * @param argCount Number of arguments passed
 * @param args Array of argument Values
 * @return The function result Value
 */
typedef Value (*NativeFn)(int argCount, Value* args);

/**
 * Represents a native function implemented in C.
 */
typedef struct {
    Obj obj;           // Base object header
    NativeFn function; // Pointer to C function
} ObjNative;

// ======================
// String Object
// ======================

/**
 * Represents a Delirium string value.
 *
 * Strings are immutable and interned for deduplication.
 */
struct ObjString {
    Obj obj;       // Base object header
    int length;    // String length in bytes (excluding null terminator)
    char* chars;   // UTF-8 character data (always null-terminated)
    uint32_t hash; // Precomputed FNV-1a hash of string contents
};

// ======================
// Object API
// ======================

/** Creates a new empty function object */
ObjFunction* newFunction();

/** Wraps a C function as a native Delirium function */
ObjNative* newNative(NativeFn function);

/**
 * Creates a new string object by copying existing characters.
 *
 * @param chars Source characters (does not take ownership)
 * @param length String length in bytes
 * @return New interned string object
 */
ObjString* copyString(char const* chars, int length);

/** Prints an object's string representation to stdout */
void printObject(Value value);

/**
 * Creates a string object taking ownership of existing characters.
 *
 * @param chars Character buffer (takes ownership)
 * @param length String length in bytes
 * @return New interned string object
 *
 * @note The buffer must be heap-allocated and null-terminated
 */
ObjString* takeString(char* chars, int length);

// ======================
// Type Checking Utility
// ======================

/**
 * Type-safe object type checking.
 *
 * @param value The Value to check
 * @param type Expected ObjType
 * @return true if the value is an object of the specified type
 */
static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif // OBJECT_H