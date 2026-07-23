#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "status.h"
#include "memory/heap/kernel_heap.h"
#include "kernel.h"

filesystem_t* filesystems[VARGOOS_MAX_FILESYSTEMS];
file_descriptor_t* file_descriptors[VARGOOS_MAX_FILE_DESCRIPTORS];

static filesystem_t** fs_get_free_filesystem(){
    int i = 0;

    for(i = 0; i < VARGOOS_MAX_FILESYSTEMS; ++i){
        if(filesystems[i] == 0){
            return &filesystems[i];
        }
    }

    return 0; //no filesystems available
}

void fs_insert_filesystem(filesystem_t* filesystem){
    filesystem_t** fs;
    
    fs = fs_get_free_filesystem();
    if(!fs){
        //panic
        print("Problem inserting a filesystem");
        while(1){}
    }

    *fs = filesystem;
}

static void fs_static_load(){
//    fs_insert_filesystem(fat16_init());
}

void fs_load(){
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init(){
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

/*
    @brief Create a new file descriptor
    @param descriptor_out pointer to the file descriptor that would be modified in this function
    @return Integer response (0 if success) anything less is an error (-3 ENOMEM)
*/

static int file_new_descriptor(file_descriptor_t** descriptor_out){
    int response = -ENOMEM;

    for(int i = 0; i < VARGOOS_MAX_FILE_DESCRIPTORS; ++i){
        if(file_descriptors[i] == 0){
            file_descriptor_t* desc = kzalloc(sizeof(file_descriptor_t));
            desc->index = i + 1; //descriptors start at 1
            file_descriptors[i] = desc;
            *descriptor_out = desc;
            response = 0;
            break;
        }
    }

    return response;
}

static file_descriptor_t* file_get_descriptor(int id){
    if(id <= 0 || id >= VARGOOS_MAX_FILE_DESCRIPTORS){
        return NULL; //invalid information
    }

    int index = id - 1;
    return file_descriptors[index];
}

filesystem_t* fs_resolve(disk_t* disk){
    filesystem_t* fs = 0;
    
    for(int i = 0; i < VARGOOS_MAX_FILESYSTEMS; ++i){
        if(filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0){
            fs = filesystems[i];
            break;
        }
    }

    return fs;
}

int fopen(const char* filename, const char* mode){
    return -EIO;
}