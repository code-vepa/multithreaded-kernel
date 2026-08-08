#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

//Bit masks (Paging flags) 9-0 bits of the page table entry structure
#define PAGING_CACHE_DISABLED   0b00010000
#define PAGING_WRITE_THROUGH    0b00001000
#define PAGING_ACCESS_FROM_ALL  0b00000100
#define PAGING_IS_WRITABLE      0b00000010
#define PAGING_IS_PRESENT       0b00000001

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

/*
This structure simply wraps the page directory pointer.
direcotry_entry points to Page Directory whose entries points to Page Tables 
whose entries point to physical frames
*/

typedef struct {
    uint32_t* directory_entry;
} paging_4gb_chunk;


paging_4gb_chunk* paging_new_4gb(uint8_t flags);
void paging_free_4gb(paging_4gb_chunk* chunk);
uint32_t* paging_4gb_chunk_get_directory(paging_4gb_chunk* chunk);
void paging_switch(paging_4gb_chunk* directory);
int paging_set(uint32_t* directory, void* virtual, uint32_t val);
int paging_is_aligned(void* address);
void enable_paging();
int paging_map_to(paging_4gb_chunk* directory, void* virtual_address,
                void* absolute_address, void* absolute_end, int flags);
int paging_map_range(paging_4gb_chunk* directory, void* virtual_address, void* absolute_address, int total_pages, int flags);
int paging_map(paging_4gb_chunk* directory, void* virtual_address, void* absolute_address, int flags);
void* paging_align_address(void* ptr);

#endif