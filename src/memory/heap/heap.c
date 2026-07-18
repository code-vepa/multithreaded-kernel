#include "heap.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"


static int heap_validate_table(void* ptr, void* end, heap_table* table){
    int response =0;

    size_t table_size = (size_t)(end - ptr);
    size_t total_blocks = table_size / VARGOOS_HEAP_BLOCK_SIZE;
    if(table->total_entries != total_blocks){
        response = -EINVARG;
        goto out;
    }

out:
    return response;
}

//This function validates the alignment and returns true
//if the alignment is fine
static int heap_validate_alignment(void* ptr){
    return ((unsigned int)ptr % VARGOOS_HEAP_BLOCK_SIZE) == 0;
}

//Returns integer: 0 - OK status
//Anything below 0 is error with the negative number representing
//an error status
//Params: Provided heap, pointer to the heap data pool, 
//pointer to the end of the heap, heap table
int heap_create(heap_chunk* heap, void* ptr, void* end, heap_table* table){

    int response = 0;
    
    if(!heap_validate_alignment(ptr) || !heap_validate_alignment(end)){
        response = -EINVARG;
        goto out;
    }

    memset(heap, 0, sizeof(heap_chunk));
    heap->start_address = ptr;
    heap->table = table;

    response = heap_validate_table(ptr, end, table);
    if(response < 0) goto out;


    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total_entries;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);
out:
    return response;
}

static uint32_t heap_align_value_to_upper(uint32_t value){
    if((value % VARGOOS_HEAP_BLOCK_SIZE) == 0) return value;

    value = (value - (value % VARGOOS_HEAP_BLOCK_SIZE));
    value += VARGOOS_HEAP_BLOCK_SIZE;
    return value;
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry){
    return entry & 0x0f; //return last 4 bits
}

//TODO: OPTIMIZE THIS FUNCTION
int heap_get_start_block(heap_chunk* heap, uint32_t total_blocks){
    heap_table* table = heap->table;
    int bc = 0; // current block
    int bs = -1; // start block

    for(size_t i = 0; i < table->total_entries; ++i){
        if(heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE){
            bc = 0;
            bs = -1;
            continue;
        }
        
        //if first block
        if(bs == -1){
            bs = i;
        }
        ++bc;

        if(bc == total_blocks){
            break;
        }
    }

    if(bs == -1){
        return -ENOMEM;
    }

    return bs;
}

//Returns the absolute address of the memory
void* heap_block_to_address(heap_chunk* heap, int block){
    return heap->start_address + (block * VARGOOS_HEAP_BLOCK_SIZE);
}

void heap_mark_taken_blocks(heap_chunk* heap, int start_block, int total_blocks){
    int end_block = (start_block + total_blocks) - 1; // end block index

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    if(total_blocks > 1){
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for(int i = start_block; i <= end_block; ++i){
        heap->table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;

        if(i != end_block -1){
            entry |= HEAP_BLOCK_HAS_NEXT;
        }
    }
}

void* heap_malloc_blocks(heap_chunk* heap, uint32_t total_blocks){
    void* address = 0;
    int start_block = heap_get_start_block(heap, total_blocks);
    if(start_block < 0) goto out;

    address = heap_block_to_address(heap, start_block);
    heap_mark_taken_blocks(heap, start_block, total_blocks);

out: 
    return address;
}

void heap_mark_free_blocks(heap_chunk* heap, int start_block){
    heap_table* table = heap->table;

    for(int i = start_block; i < (int)table->total_entries; ++i){
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if(!(entry & HEAP_BLOCK_HAS_NEXT)){
            break;
        }
    }
}

int heap_address_to_block(heap_chunk* heap, void* address){
    return ((int) (address - heap->start_address)) / VARGOOS_HEAP_BLOCK_SIZE; 
}

void* heap_malloc(heap_chunk* heap, size_t size){
    
    size_t aligned_size = heap_align_value_to_upper(size);
    uint32_t total_blocks = aligned_size / VARGOOS_HEAP_BLOCK_SIZE;

    return heap_malloc_blocks(heap, total_blocks);
}

void heap_free(heap_chunk* heap, void* ptr){

    heap_mark_free_blocks(heap, heap_address_to_block(heap, ptr));
}
