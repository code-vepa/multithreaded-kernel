#ifndef DISK_H
#define DISK_H

#include "filesystem/file.h"

typedef unsigned int VARGOOS_DISK_TYPE;

//real physical hard disk
#define VARGOOS_DISK_TYPE_REAL 0


typedef struct disk{
    VARGOOS_DISK_TYPE type;
    int sector_size;
    int id; // Disk id
    filesystem_t* filesystem;
    void* fs_private; // Private data of the filesystem
} disk_t;

void disk_search_init();
disk_t* get_disk(int index);
int disk_read_block(disk_t* idisk, unsigned int lba, int total, void* buffer);

#endif