#ifndef DISK_H
#define DISK_H

typedef unsigned int VARGOOS_DISK_TYPE;

//real physical hard disk
#define VARGOOS_DISK_TYPE_REAL 0


struct disk{
    VARGOOS_DISK_TYPE type;
    int sector_size;
}typedef disk_t;

void disk_search_init();
disk_t* disk_get(int index);
int disk_read_block(disk_t* idisk, unsigned int lba, int total, void* buffer);

#endif