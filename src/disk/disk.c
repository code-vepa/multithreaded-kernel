#include "io/io.h"
#include "disk.h"
#include "memory/memory.h"
#include "config.h"
#include "status.h"

disk_t disk;

/*
    osdev ATA read/write sectors

    @name: disk_read_sector
    @brief: this function will read the sector from the primary hard disk

    @param: int lba, the number of blocks and pointer to the buffer
    @return: int type (returns 0 on success)
*/

int disk_read_sector(int lba, int total_blocks, void* buffer){
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F2, total_blocks);
    outb(0x1F3, (unsigned char) (lba & 0xFF));
    outb(0x1F4, (unsigned char) (lba >> 8));
    outb(0x1F5, (unsigned char) (lba >> 16));
    outb(0x1F7, 0x20);
    
    unsigned short * ptr = (unsigned short *) buffer;

    for(int i = 0; i < total_blocks; ++i){
        //wait for the buffer
        char c = insb(0x1F7);

        while(!(c & 0x08)){
            c = insb(0x1F7);
        }
        
        // copy from hard disk to memory
        for(int j = 0; j < 256; ++j){
            *ptr = insw(0x1F0);
            ++ptr;
        }
     }

    return 0;
}


/*
    @name: disk_search_init
    @brief: this function is responsible for searching for disks AND initializing them

    this is an abstraction
*/

void disk_search_init(){
    memset(&disk, 0, sizeof(disk));
    disk.type = VARGOOS_DISK_TYPE_REAL;
    disk.sector_size = VARGOOS_SECTOR_SIZE;
    disk.id = 0;
    disk.filesystem = fs_resolve(&disk);
}

disk_t* disk_get(int index){
    if(index != 0) 
        return 0;

    return &disk;
}

int disk_read_block(disk_t* idisk, unsigned int lba, int total, void* buffer){
    
    if(idisk != &disk){
        return -EIO;
    }

    return disk_read_sector(lba, total, buffer);
}