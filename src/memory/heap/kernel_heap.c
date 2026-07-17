#include "kernel_heap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"

heap_chunk kernel_heap;
heap_table kernel_heap_table;


void kernel_heap_init(){
    int total_table_entries = VARGOOS_HEAP_SIZE_BYTES / VARGOOS_HEAP_BLOCK_SIZE;
    kernel_heap_table.entries= (HEAP_BLOCK_TABLE_ENTRY*) VARGOOS_HEAP_TABLE_ADDRESS;
    kernel_heap_table.total_entries = total_table_entries;

    void* end = (void*)( VARGOOS_HEAP_ADDRESS + VARGOOS_HEAP_SIZE_BYTES );
    int response = heap_create(&kernel_heap, (void*) VARGOOS_HEAP_ADDRESS, end, &kernel_heap_table);
    
    if(response < 0){
        print("FAILED TO CREATE HEAP\n");
    }
}