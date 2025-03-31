#include <cstdlib>  // For free/realloc
#include <iostream> // For input/output operations
#include <memory.h> // For memory operations

#include "memory.h" // For memory management interface
#include "object.h" // For object type definitions
#include "vm.h"     // For VM object list access

/**
 * Core memory management function that handles all allocations,
 * reallocations, and deallocations in the VM.
 *
 * @param pointer Existing memory block (may be NULL)
 * @param oldSize Current size of memory block (0 for new allocations)
 * @param newSize Desired size (0 for deallocations)
 * @return Pointer to allocated memory, or NULL if freeing
 *
 * @note Uses realloc() for underlying operations
 * @note Exits program if allocation fails (out of memory)
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize)
{
    // Handle deallocations
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    // Perform allocation/reallocation
    void* result = realloc(pointer, newSize);

    // Out-of-memory handling
    if (result == NULL) {
        std::cerr << "[Delirium] Memory allocation failed" << std::endl;
        exit(1); // Hard exit on allocation failure
    }

    return result;
}

/**
 * Frees a single object and its associated resources.
 *
 * @param object The object to free
 *
 * @note Handles type-specific cleanup:
 *   - Functions: Frees their bytecode chunks
 *   - Strings: Frees character buffers
 *   - All objects: Removes from memory
 */
static void freeObject(Obj* object)
{
    switch (object->type) {
    case OBJ_FUNCTION: {
        ObjFunction* function = (ObjFunction*)object;
        // Release function bytecode
        freeChunk(&function->chunk);
        // Free function object itself
        FREE(ObjFunction, object);
        break;
    }

    case OBJ_NATIVE:
        // Native functions have no extra resources
        FREE(ObjNative, object);
        break;

    case OBJ_STRING: {
        ObjString* string = (ObjString*)object;
        // Free string's character buffer (+1 for null terminator)
        FREE_ARRAY(char, string->chars, string->length + 1);
        // Free string object itself
        FREE(ObjString, object);
        break;
    }
    }
}

/**
 * Frees all objects in the VM's object pool.
 *
 * @note Walks the entire linked list of objects
 * @note Called during VM shutdown
 * @note Handles all object types via freeObject()
 */
void freeObjects()
{
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next; // Save next pointer before freeing
        freeObject(object);
        object = next;
    }
}