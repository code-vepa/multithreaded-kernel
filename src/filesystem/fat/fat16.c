#include "fat16.h"
#include "status.h"
#include "string/string.h"

int fat16_resolve(disk_t* disk);
void* fat16_open(disk_t* disk, path_part_t* path, FILE_MODE mode);

filesystem_t fat16_fs = {
    .resolve = fat16_resolve,
    .open = fat16_open
};

filesystem_t* fat16_init(){
    strcpy(fat16_fs.name, "FAT16");
    return 0;
}

int fat16_resolve(disk_t* disk){
    return -EIO;
}

void* fat16_open(disk_t* disk, path_part_t* path, FILE_MODE mode){
    return 0;
}