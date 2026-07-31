#ifndef FILE_H
#define FILE_H

#include "path_parser.h"


typedef unsigned int FILE_SEEK_MODE;
enum{
    SEEK_SET, SEEK_CURRENT, SEEK_END
};

typedef unsigned int FILE_MODE;
enum{
    FILE_MODE_READ, FILE_MODE_WRITE, FILE_MODE_APPEND, FILE_MODE_INVALID
};

typedef struct disk disk_t;

typedef void*(*FS_OPEN_FUNCTION)(disk_t* disk, path_part_t* part, FILE_MODE mode);
typedef int(*FS_RESOLVE_FUNCTION)(disk_t* disk);

typedef struct filesystem{
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    char name[20];
} filesystem_t;

typedef struct file_descriptor{
    int index; //desc index
    filesystem_t* filesystem;
    void* private; //private data for the internal file descriptor
    disk_t* disk; //the disk that filesystem will be used on
} file_descriptor_t;

void fs_init();
int fopen(const char* filename, const char* mode_string);
void fs_insert_filesystem(filesystem_t* filesystem);
filesystem_t* fs_resolve(disk_t* disk);

#endif