#ifndef COMMON_H
#define COMMON_H

// ======================
// Standard Library Includes
// ======================

#include <cstddef> // For size_t, NULL, etc.
#include <cstdint> // For fixed-width integer types (uint8_t, etc.)

// ======================
// Debug Configuration
// ======================

/**
 * @def DEBUG_PRINT_CODE
 * When defined, enables disassembly of generated bytecode during compilation.
 * Useful for debugging the compiler output.
 */
// #define DEBUG_PRINT_CODE

/**
 * @def DEBUG_TRACE_EXECUTION
 * When defined, enables VM execution tracing (stack dumps and instruction tracing).
 * Significant performance impact, use only for debugging.
 */
// #define DEBUG_TRACE_EXECUTION

/**
 * @def DEBUG_MUTATE_CODE
 * When defined, enables code mutations when an error ocurrs.
 * This may be turned off when debugging
 */
#define DEBUG_MUTATE_CODE

// ======================
// VM Constants
// ======================

/**
 * @def UINT8_COUNT
 * The number of possible 8-bit unsigned integer values (256).
 * Used for sizing arrays that need to index all possible byte values.
 *
 * Value breakdown:
 * - UINT8_MAX = 255 (maximum 8-bit unsigned value)
 * - +1 to include all values (0-255 inclusive)
 */
#define UINT8_COUNT (UINT8_MAX + 1)

#endif // COMMON_H