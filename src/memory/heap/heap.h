#ifndef HEAP_H
#define HEAP_H
#include "config.h"
#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01 //lower bit (1) = taken
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0X00

#define HEAP_BLOCK_HAS_NEXT     0B10000000
#define HEAP_BLOCK_IS_FREE      0b01000000

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

typedef struct{
    HEAP_BLOCK_TABLE_ENTRY* entries;
    size_t total_entries;
}  heap_table;

typedef struct{
    struct heap_table* table_ptr;
    void* start_address;
}  heap_chunk;

int heap_create(heap_chunk* heap, void* ptr, void* end, heap_table* table);

#endif