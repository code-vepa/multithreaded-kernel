#include "paging.h"
#include "memory/heap/kernel_heap.h"
#include "status.h"

void paging_load_directory(uint32_t* directory);
static uint32_t* current_directory = 0;


/*
@name paging_new_4gb
@brief creates a new 4 GB mapped paging structure

@param flags Paging flags

@return pointer to the paging_4gb_chunk structure containing the page directory
*/

paging_4gb_chunk* paging_new_4gb(uint8_t flags){

    //4 * 1024 
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;

    for(int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; ++i){
        //4 * 1024 entries
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        
        for(int j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; ++j){
            entry[j] = (offset + (j * PAGING_PAGE_SIZE)) | flags;
        }

        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        directory[i] = (uint32_t) entry | flags | PAGING_IS_WRITABLE;
    }

    paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

/*
    @name: paging_switch
    @brief: switch the CPU to the different page directory
            loads passed directory to cr3 register
            This function is implemented in assembly paging.asm file

    @param: 32 bit unsigned integer pointer to the directory

    @return: void type
*/
void paging_switch(uint32_t* directory){
    paging_load_directory(directory);
    current_directory = directory;
}

uint32_t* paging_4gb_chunk_get_directory(paging_4gb_chunk* chunk){
    return chunk->directory_entry;
}

int paging_is_aligned(void* address){
    return ((uint32_t)address % PAGING_PAGE_SIZE) == 0;
}

/*
    @name: get_paging_indices
    @brief: this will resolve the virtual address to a directory and table indecies
            every page table is aligned to 4096

    @math: directory_index = virtual_address / (1024 * 4096) 
           (divide the virtual address by the total size of the virutal table)

           table_index = virtual_address % (1024 * 4096) / 4096
           this equation returns the table index

    @param: the virtual address that needs to be resolved, directory & table indeces out

    @return: integer status from status.h

*/

int get_paging_indeces(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out){
    int response = STATUS_OK;
    
    if(!paging_is_aligned(virtual_address)){
        response = -EINVARG;
        goto out;
    }

    //1024 * 4096
    *directory_index_out = ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t) virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);

out:
    return response;
}

/*
    @name: paging_set
    @brief: changes one page table entry
        updates the mappping for a single virtual page by modifying the appropriate page table entry
    
    @params: pointer to the directory, vitrual address and value

    @return: integer status (from status.h)
*/

int paging_set(uint32_t* directory, void* virtual, uint32_t val){
    if(!paging_is_aligned(virtual)){
        return -EINVARG;        
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    int response = get_paging_indeces(virtual, &directory_index, &table_index);

    if(response < 0) return response;

    uint32_t entry = directory[directory_index];
    //Bits (31-11 = 20 bits) after extraction: 0xfffff000
    uint32_t* table = (uint32_t*)(entry & 0xfffff000);
    table[table_index] = val;

    return STATUS_OK;
}