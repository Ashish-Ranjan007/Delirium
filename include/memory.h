#ifndef MEMORY_H
#define MEMORY_H

#include "common.h" // For size_t and other basic types
#include "object.h" // For object-related memory operations

// ======================
// Memory Management Macros
// ======================

/**
 * Calculates new capacity when growing dynamic arrays.
 * Uses exponential growth strategy:
 * - Starts with minimum 8 elements
 * - Doubles size when growing
 *
 * @param capacity Current array capacity
 * @return New increased capacity
 */
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

/**
 * Grows or shrinks a dynamic array with proper type safety.
 *
 * @param type Element type
 * @param pointer Current array pointer
 * @param oldCount Current element count
 * @param newCount Desired element count
 * @return Pointer to resized array
 *
 * @note Handles both allocation and deallocation
 * @note Uses reallocate() for actual memory operations
 */
#define GROW_ARRAY(type, pointer, oldCount, newCount)     \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

/**
 * Frees a dynamically allocated array.
 *
 * @param type Element type
 * @param pointer Array to free
 * @param oldCount Current element count (for size calculation)
 */
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

/**
 * Allocates new memory for objects.
 *
 * @param type Type to allocate
 * @param count Number of elements
 * @return Pointer to new memory
 */
#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

/**
 * Frees a single object allocation.
 *
 * @param type Type of object
 * @param pointer Object to free
 */
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

// ======================
// Core Memory Functions
// ======================

/**
 * Central memory management function that handles:
 * - Allocations (when oldSize = 0, newSize > 0)
 * - Reallocations (when oldSize > 0, newSize > 0)
 * - Deallocations (when newSize = 0)
 *
 * @param pointer Existing memory block or NULL
 * @param oldSize Current allocation size in bytes
 * @param newSize Desired new size in bytes
 * @return Pointer to allocated memory (NULL if freeing)
 *
 * @note All memory operations should go through this function
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

/**
 * Frees all allocated objects in the VM's object pool.
 *
 * @note Called during VM shutdown
 * @note Walks the entire object linked list
 */
void freeObjects();

#endif // MEMORY_H