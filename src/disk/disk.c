#include "io/io.h"
#include "disk.h"
#include "memory/memory.h"
#include "config.h"
#include "status.h"

disk_t disk;

/*
    @brief: This function will read the sector from the primary hard disk
    @return: int type (returns 0 on success)
*/
int disk_read_sector(int lba, int total_blocks, void* buffer){
    // Master drive/LBA mode
    outb(0x1F6, (lba >> 24) | 0xE0); // Sets up drives and LBA registers
    outb(0x1F2, total_blocks); // Tells compiler how many sectors to read

    // Send the rest of LBA
    outb(0x1F3, (unsigned char) (lba & 0xFF)); // LBA bits 0-7
    outb(0x1F4, (unsigned char) (lba >> 8)); // LBA bits 8-15
    outb(0x1F5, (unsigned char) (lba >> 16)); // LBA bits 16-23
    outb(0x1F7, 0x20); // Send the read command
    
    unsigned short * ptr = (unsigned short *) buffer;

    for(int i = 0; i < total_blocks; ++i){
        // Wait for the buffer to be ready
        char c = insb(0x1F7);
        
        // Read data, sector by sector
        while(!(c & 0x08)){
            c = insb(0x1F7);
        }
        
        // Copy from hard disk to memory, for every requester sector
        // 256 words -> 512 bytes
        for(int j = 0; j < 256; ++j){
            *ptr = insw(0x1F0);
            ++ptr;
        }
     }

    return 0;
}


/*
    @brief: This function is responsible for searching for disks AND initializing them

    this is an abstraction
*/
void disk_search_init(){
    memset(&disk, 0, sizeof(disk));
    disk.type = VARGOOS_DISK_TYPE_REAL;
    disk.sector_size = VARGOOS_SECTOR_SIZE;
    disk.id = 0;
    disk.filesystem = fs_resolve(&disk);
}

disk_t* get_disk(int index){
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