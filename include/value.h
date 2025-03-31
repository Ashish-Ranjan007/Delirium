#ifndef VALUE_H
#define VALUE_H

// ======================
// Forward Declarations
// ======================

/* Object base type (defined in object.h) */
typedef struct Obj Obj;

/* String object type (defined in object.h) */
typedef struct ObjString ObjString;

// ======================
// Value Type System
// ======================

/**
 * Fundamental value types in Delirium's dynamic type system.
 * The VM uses tagged unions to represent all possible value types.
 */
typedef enum ValueType {
    VAL_BOOL,   // Boolean values (true/false)
    VAL_NIL,    // Nil/null value
    VAL_NUMBER, // Double-precision floating point numbers
    VAL_OBJ     // Heap-allocated objects (strings, functions, etc)
} ValueType;

// ======================
// Value Representation
// ======================

/**
 * Represents a Delirium value in the VM.
 * Uses a tagged union to store different types efficiently.
 */
typedef struct Value {
    ValueType type; // Tag identifying the value type
    union {
        bool boolean;  // VAL_BOOL
        double number; // VAL_NUMBER
        Obj* obj;      // VAL_OBJ (pointer to heap-allocated object)
    } as;              // Named union for type-safe access
} Value;

// ======================
// Value Type Predicates
// ======================

/** Checks if a Value is boolean */
#define IS_BOOL(value) ((value).type == VAL_BOOL)

/** Checks if a Value is nil */
#define IS_NIL(value) ((value).type == VAL_NIL)

/** Checks if a Value is a number */
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

/** Checks if a Value is an object */
#define IS_OBJ(value) ((value).type == VAL_OBJ)

// ======================
// Value Conversion Macros
// ======================

/** Extracts boolean value (asserts type is VAL_BOOL) */
#define AS_BOOL(value) ((value).as.boolean)

/** Extracts number value (asserts type is VAL_NUMBER) */
#define AS_NUMBER(value) ((value).as.number)

/** Extracts object pointer (asserts type is VAL_OBJ) */
#define AS_OBJ(value) ((value).as.obj)

// ======================
// Value Construction Macros
// ======================

/** Creates a boolean Value */
#define BOOL_VAL(value) ((Value) { VAL_BOOL, { .boolean = value } })

/** Creates a nil Value */
#define NIL_VAL ((Value) { VAL_NIL, { .number = 0 } })

/** Creates a number Value */
#define NUMBER_VAL(value) ((Value) { VAL_NUMBER, { .number = value } })

/** Creates an object Value (takes ownership of pointer) */
#define OBJ_VAL(object) ((Value) { VAL_OBJ, { .obj = (Obj*)object } })

// ======================
// Value Array Structure
// ======================

/**
 * Dynamic array of Values used for:
 * - Constant pools in chunks
 * - Temporary value storage
 */
typedef struct ValueArray {
    int capacity;  // Allocated storage size
    int count;     // Number of elements in use
    Value* values; // Contiguous array of Values
} ValueArray;

// ======================
// Value Operations API
// ======================

/**
 * Compares two Values for equality following Delirium's rules:
 * - Different types are never equal
 * - Objects are compared by value (not identity)
 * - Numbers follow IEEE equality
 * - Booleans and nil compare normally
 */
bool valuesEqual(Value a, Value b);

/** Initializes an empty ValueArray */
void initValueArray(ValueArray* array);

/** Appends a Value to the array (grows if needed) */
void writeValueArray(ValueArray* array, Value value);

/** Releases all memory used by a ValueArray */
void freeValueArray(ValueArray* array);

/**
 * Prints a Value to stdout according to its type:
 * - Numbers: decimal format
 * - Booleans: "true"/"false"
 * - Nil: "nil"
 * - Objects: type-specific formatting
 */
void printValue(Value value);

#endif // VALUE_H