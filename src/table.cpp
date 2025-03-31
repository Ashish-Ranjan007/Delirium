#include <cstdlib> // For NULL
#include <cstring> // For memcmp

#include "memory.h" // For memory allocation macros
#include "object.h" // For ObjString definition
#include "table.h"  // For Table and Entry declarations
#include "value.h"  // For Value type

// Maximum load factor before resizing the table (75%)
#define TABLE_MAX_LOAD 0.75

/**
 * Initializes an empty hash table.
 *
 * @param table Pointer to uninitialized Table structure
 */
void initTable(Table* table)
{
    table->count = 0;      // No entries stored
    table->capacity = 0;   // No storage allocated
    table->entries = NULL; // Entries array pointer
}

/**
 * Releases all memory used by the hash table.
 *
 * @param table Table to deallocate
 *
 * @note Resets the table to empty state after freeing
 */
void freeTable(Table* table)
{
    // Free the entries array
    FREE_ARRAY(Entry, table->entries, table->capacity);
    // Reset to initial empty state
    initTable(table);
}

/**
 * Finds an entry for the given key in the hash table.
 *
 * @param entries Pointer to entries array
 * @param capacity Size of the entries array
 * @param key String key to search for
 * @return Pointer to the found entry or an empty slot
 *
 * @note Uses linear probing for collision resolution
 * @note Handles tombstone entries during search
 */
static Entry* findEntry(Entry* entries, int capacity,
    ObjString* key)
{
    // Start at initial bucket position
    uint32_t index = key->hash % capacity;
    Entry* tombstone = NULL; // First tombstone encountered

    for (;;) {
        Entry* entry = &entries[index];

        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // Empty entry (not tombstone)
                return tombstone != NULL ? tombstone : entry;
            } else {
                // Found a tombstone entry
                if (tombstone == NULL)
                    tombstone = entry;
            }
        } else if (entry->key == key) {
            // Found exact key match
            return entry;
        }

        // Move to next slot (with wrapping)
        index = (index + 1) % capacity;
    }
}

/**
 * Looks up a value by key in the hash table.
 *
 * @param table Table to search
 * @param key Key to look up
 * @param value [out] Receives the value if found
 * @return true if key was found, false otherwise
 */
bool tableGet(Table* table, ObjString* key, Value* value)
{
    // Empty table can't contain anything
    if (table->count == 0)
        return false;

    // Find the entry for the key
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false; // Key not found

    // Return the found value
    *value = entry->value;
    return true;
}

/**
 * Resizes the hash table to the new capacity.
 *
 * @param table Table to resize
 * @param capacity New capacity (must be larger than current)
 *
 * @note Rehashes all existing entries
 * @note Ignores tombstones during rehashing
 */
static void adjustCapacity(Table* table, int capacity)
{
    // Allocate new empty entries array
    Entry* entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    // Rehash existing entries into new array
    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL)
            continue; // Skip empty slots and tombstones

        // Find position in new array
        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    // Free old entries array
    FREE_ARRAY(Entry, table->entries, table->capacity);

    // Update table with new storage
    table->entries = entries;
    table->capacity = capacity;
}

/**
 * Inserts or updates a key-value pair in the table.
 *
 * @param table Table to modify
 * @param key Key to insert/update
 * @param value Value to associate with key
 * @return true if new key was added, false if existing key was updated
 *
 * @note Automatically grows table if load factor exceeds TABLE_MAX_LOAD
 */
bool tableSet(Table* table, ObjString* key, Value value)
{
    // Grow table if needed
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    // Find or create entry for the key
    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;

    // Only increment count if not replacing tombstone
    if (isNewKey && IS_NIL(entry->value))
        table->count++;

    // Update the entry
    entry->key = key;
    entry->value = value;
    return isNewKey;
}

/**
 * Removes a key-value pair from the table.
 *
 * @param table Table to modify
 * @param key Key to remove
 * @return true if key was found and removed, false otherwise
 *
 * @note Leaves a tombstone entry to maintain probe sequences
 */
bool tableDelete(Table* table, ObjString* key)
{
    // Empty table can't contain anything
    if (table->count == 0)
        return false;

    // Find the entry
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false; // Key not found

    // Replace with tombstone (key=NULL, value=true)
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

/**
 * Copies all entries from one table to another.
 *
 * @param from Source table to copy from
 * @param to Destination table to copy to
 *
 * @note Existing entries in destination may be overwritten
 */
void tableAddAll(Table* from, Table* to)
{
    // Process all slots in source table
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        // Only copy actual entries (skip empty/tombstones)
        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

/**
 * Finds an interned string in the table by content.
 *
 * @param table Table to search
 * @param chars String characters to match
 * @param length Length of string in bytes
 * @param hash Precomputed hash of the string
 * @return Existing ObjString if found, NULL otherwise
 *
 * @note Used for string interning/deduplication
 */
ObjString* tableFindString(Table* table, char const* chars,
    int length, uint32_t hash)
{
    // Empty table can't contain anything
    if (table->count == 0)
        return NULL;

    // Start at initial bucket position
    uint32_t index = hash % table->capacity;
    for (;;) {
        Entry* entry = &table->entries[index];

        if (entry->key == NULL) {
            // Stop if we find an empty non-tombstone entry
            if (IS_NIL(entry->value))
                return NULL;
        } else if (entry->key->length == length && entry->key->hash == hash && memcmp(entry->key->chars, chars, length) == 0) {
            // Found matching string
            return entry->key;
        }

        // Move to next slot (with wrapping)
        index = (index + 1) % table->capacity;
    }
}