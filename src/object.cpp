#include <cstdint>
#include <cstring>
#include <iostream>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

// Macro to allocate objects of specific types
// type: The C struct type to allocate
// objectType: The ObjType enum value for this object
#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

// Allocates a new object and adds it to the VM's object list
// size: Size in bytes of the object to allocate
// type: The type tag for the object (ObjType enum)
// Returns: Pointer to newly allocated object
static Obj* allocateObject(size_t size, ObjType type)
{
    // Allocate raw memory for the object
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    // Insert the object at the head of the VM's object list
    object->next = vm.objects;
    vm.objects = object;

    return object;
}

// Creates a new function object with default values:
// - arity = 0 (no parameters)
// - name = NULL (anonymous)
// - empty bytecode chunk
// Returns: Pointer to new ObjFunction
ObjFunction* newFunction()
{
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

// Creates a new native function object
// function: Pointer to the native C function
// Returns: Pointer to new ObjNative
ObjNative* newNative(NativeFn function)
{
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

// Allocates a string object and adds it to the string table
// chars: The character data (will be owned by the string)
// length: Length of the string (not including null terminator)
// hash: Precomputed hash of the string
// Returns: Pointer to new ObjString
static ObjString* allocateString(char* chars, int length, uint32_t hash)
{
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    // Add the string to the VM's string interning table
    tableSet(&vm.strings, string, NIL_VAL);

    return string;
}

// Computes a 32-bit FNV-1a hash for a string
// key: Pointer to string data
// length: Length of string in bytes
// Returns: 32-bit hash value
static uint32_t hashString(char const* key, int length)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint32_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

// Creates a string object, taking ownership of the provided chars buffer
// chars: Character buffer to take ownership of (must be heap allocated)
// length: Length of the string
// Returns: Pointer to new or existing interned string
ObjString* takeString(char* chars, int length)
{
    // Compute the string's hash
    uint32_t hash = hashString(chars, length);

    // Check if string already exists in intern table
    ObjString* interned = tableFindString(
        &vm.strings,
        chars,
        length,
        hash);
    if (interned != NULL) {
        // Free the duplicate buffer
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    // Create new string object
    return allocateString(chars, length, hash);
}

// Creates a string object by copying the provided characters
// chars: Source characters to copy
// length: Length of string to copy
// Returns: Pointer to new or existing interned string
ObjString* copyString(char const* chars, int length)
{
    // Compute the string's hash
    uint32_t hash = hashString(chars, length);

    // Check if string already exists in intern table
    ObjString* interned = tableFindString(
        &vm.strings,
        chars,
        length,
        hash);
    if (interned != NULL)
        return interned;

    // Allocate and copy the string data (+1 for null terminator)
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    // Create new string object
    return allocateString(heapChars, length, hash);
}

// Prints a function object's representation
// function: The function object to print
static void printFunction(ObjFunction* function)
{
    if (function->name == NULL) {
        printf("<script>"); // Anonymous function
        return;
    }
    printf("<fn %s>", function->name->chars); // Named function
}

// Prints any object's string representation
// value: The object value to print (must be an object type)
void printObject(Value value)
{
    switch (OBJ_TYPE(value)) {
    case OBJ_FUNCTION:
        printFunction(AS_FUNCTION(value));
        break;
    case OBJ_NATIVE:
        printf("<native fn>"); // Native function
        break;
    case OBJ_STRING:
        std::cout << AS_CSTRING(value); // String object
        break;
    }
}