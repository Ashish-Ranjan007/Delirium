#include <cstring>  // For string operations
#include <iostream> // For output operations
#include <numbers>  // For number constants

#include "memory.h" // For memory management macros
#include "object.h" // For object printing
#include "value.h"  // For Value type definitions

/**
 * Initializes an empty ValueArray.
 *
 * @param array Pointer to uninitialized ValueArray
 *
 * @note Sets all fields to zero/NULL state
 */
void initValueArray(ValueArray* array)
{
    array->values = NULL; // No storage allocated
    array->capacity = 0;  // No capacity
    array->count = 0;     // No elements stored
}

/**
 * Appends a value to the dynamic array.
 *
 * @param array Array to modify
 * @param value Value to append
 *
 * @note Automatically grows the array if needed
 * @note Uses exponential growth strategy
 */
void writeValueArray(ValueArray* array, Value value)
{
    // Check if we need to grow the array
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        // Allocate larger array and copy old values
        array->values = GROW_ARRAY(
            Value,
            array->values,
            oldCapacity,
            array->capacity);
    }

    // Store the new value
    array->values[array->count] = value;
    array->count++;
}

/**
 * Releases all memory used by a ValueArray.
 *
 * @param array Array to deallocate
 *
 * @note Resets the array to empty state after freeing
 */
void freeValueArray(ValueArray* array)
{
    // Free the storage
    FREE_ARRAY(Value, array->values, array->capacity);
    // Reset to empty state
    initValueArray(array);
}

/**
 * Prints a Value's string representation to stdout.
 *
 * @param value Value to print
 *
 * @note Handles all value types:
 *   - Booleans: "true"/"false"
 *   - Nil: "nil"
 *   - Numbers: %g format
 *   - Objects: Delegates to printObject()
 */
void printValue(Value value)
{
    switch (value.type) {
    case VAL_BOOL:
        printf(AS_BOOL(value) ? "true" : "false");
        break;
    case VAL_NIL:
        printf("nil");
        break;
    case VAL_NUMBER:
        printf("%g", AS_NUMBER(value));
        break;
    case VAL_OBJ:
        printObject(value);
        break;
    }
}

/**
 * Compares two Values for equality.
 *
 * @param a First value to compare
 * @param b Second value to compare
 * @return true if values are equal, false otherwise
 *
 * @note Follows Delirium's equality rules:
 *   - Different types are never equal
 *   - Objects compared by identity
 *   - Numbers compared by numeric value
 *   - Booleans and nil compared normally
 */
bool valuesEqual(Value a, Value b)
{
    // Different types can never be equal
    if (a.type != b.type)
        return false;

    // Type-specific comparison
    switch (a.type) {
    case VAL_BOOL:
        return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:
        return true; // All nil values are equal
    case VAL_NUMBER:
        return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ:
        return AS_OBJ(a) == AS_OBJ(b); // Pointer comparison
    default:
        return false; // Unreachable for valid Values
    }
}