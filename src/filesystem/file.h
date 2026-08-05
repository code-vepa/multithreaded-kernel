#ifndef FILE_H
#define FILE_H

#include "path_parser.h"
#include <stdint.h>

typedef unsigned int FILE_SEEK_MODE;
enum{
    SEEK_SET, SEEK_CURRENT, SEEK_END
};

typedef unsigned int FILE_MODE;
enum{
    FILE_MODE_READ, FILE_MODE_WRITE, FILE_MODE_APPEND, FILE_MODE_INVALID
};

typedef unsigned int FILE_STAT_FLAGS;
enum{
    FILE_STAT_READ_ONLY = 0b00000001
};

typedef struct disk disk_t;
struct file_stat;

typedef void*(*FS_OPEN_FUNCTION)(disk_t* disk, path_part_t* part, FILE_MODE mode);
typedef int(*FS_READ_FUNCTION)(disk_t* disk, void* private, uint32_t size, uint32_t nmemb, char* out);
typedef int(*FS_RESOLVE_FUNCTION)(disk_t* disk);
typedef int(*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE mode);
typedef int(*FS_STAT_FUNCTION)(disk_t* disk, void* private, struct file_stat* stat);
typedef int(*FS_CLOSE_FUNCTION)(void* private);

typedef struct filesystem{
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;
    FS_CLOSE_FUNCTION close;
    char name[20];
} filesystem_t;

typedef struct file_descriptor{
    int index; //desc index
    filesystem_t* filesystem;
    void* private; //private data for the internal file descriptor
    disk_t* disk; //the disk that filesystem will be used on
} file_descriptor_t;

struct file_stat{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
};


void fs_init();
int fopen(const char* filename, const char* mode_string);
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd);
int fseek(int fd, int offset, FILE_SEEK_MODE mode);
int fstat(int fd, struct file_stat* stat);
int fclose(int fd);
void fs_insert_filesystem(filesystem_t* filesystem);
filesystem_t* fs_resolve(disk_t* disk);

#endif