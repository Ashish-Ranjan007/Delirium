#ifndef TABLE_H
#define TABLE_H

#include "common.h" // For basic type definitions
#include "value.h"  // For Value type

// ======================
// Hash Table Entry
// ======================

/**
 * Represents a single key-value pair in the hash table.
 *
 * The table uses open addressing with linear probing for collision resolution.
 * Tombstone entries (key=NULL, value=NIL) mark deleted slots.
 */
typedef struct Entry {
    ObjString* key; // String key (interned/unique strings only)
    Value value;    // Associated value (any Delirium type)
} Entry;

// ======================
// Hash Table Structure
// ======================

/**
 * Dynamic hash table implementation used for:
 * - Global variables
 * - String interning
 * - Object properties
 *
 * Implements a key-value store with O(1) average case operations.
 */
typedef struct Table {
    int count;      // Number of active entries (excluding tombstones)
    int capacity;   // Total allocated slots in entries array
    Entry* entries; // Array of slots (size = capacity)
} Table;

// ======================
// Table Management API
// ======================

/**
 * Initializes an empty hash table.
 *
 * @param table Uninitialized table to prepare for use
 */
void initTable(Table* table);

/**
 * Releases all memory used by a hash table.
 *
 * @param table Table to deallocate (does not free keys/values)
 */
void freeTable(Table* table);

// ======================
// Core Operations
// ======================

/**
 * Inserts or updates a key-value pair.
 *
 * @param table Target table
 * @param key String key (must be interned ObjString)
 * @param value Value to associate with key
 * @return true if new entry was created, false if existing entry was updated
 *
 * @note Grows table automatically if load factor exceeds 75%
 */
bool tableSet(Table* table, ObjString* key, Value value);

/**
 * Copies all entries from one table to another.
 *
 * @param from Source table
 * @param to Destination table (existing entries may be overwritten)
 */
void tableAddAll(Table* from, Table* to);

/**
 * Looks up a value by key.
 *
 * @param table Table to search
 * @param key Key to find
 * @param value [out] If found, receives the associated value
 * @return true if key was found, false otherwise
 */
bool tableGet(Table* table, ObjString* key, Value* value);

/**
 * Removes a key-value pair from the table.
 *
 * @param table Table to modify
 * @param key Key to remove
 * @return true if entry was found and removed, false otherwise
 *
 * @note Leaves a tombstone entry to maintain probe sequences
 */
bool tableDelete(Table* table, ObjString* key);

// ======================
// String Interning
// ======================

/**
 * Finds an interned string in the table by content.
 *
 * @param table String intern table
 * @param chars Raw string characters
 * @param length String length in bytes
 * @param hash Precomputed FNV-1a hash of the string
 * @return Existing ObjString if found, NULL otherwise
 *
 * @note Used for string deduplication (interning)
 */
ObjString* tableFindString(Table* table, char const* chars, int length, uint32_t hash);

#endif // TABLE_H