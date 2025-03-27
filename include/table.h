#ifndef TABLE_H
#define TABLE_H

#include "common.h"
#include "value.h"

typedef struct Entry {
    ObjString* key;
    Value value;
} Entry;

typedef struct Table {
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
bool tableSet(Table* table, ObjString* key, Value value);
void tableAddAll(Table* from, Table* to);
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableDelete(Table* table, ObjString* key);
ObjString* tableFindString(Table* table, char const* chars, int length, uint32_t hash);

#endif // !TABLE_H
