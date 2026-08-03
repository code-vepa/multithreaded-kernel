#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "status.h"
#include "memory/heap/kernel_heap.h"
#include "kernel.h"
#include "fat/fat16.h"
#include "disk/disk.h"
#include "string/string.h"


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
    fs_insert_filesystem(fat16_init());
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


FILE_MODE get_file_mode_by_string(const char* str){
	FILE_MODE mode = FILE_MODE_INVALID;
	
	if(strncmp(str, "r", 1) == 0){
		mode = FILE_MODE_READ;
	}
	else if(strncmp(str, "w", 1) == 0){
		mode = FILE_MODE_WRITE;
	}
	else if(strncmp(str, "a", 1) == 0){
		mode = FILE_MODE_APPEND;
	}
	
	return mode;
}

/*
    step 0. fopen resolves the path by dividing into path root and path part
    step 1. get the current disk (client provides disk drive number e.g 0:/)
    step 2. check if disk doesn't have a fs in it (if doesn't then we cannot resolve)
    step 3. retrieve mode
    step 4. if it is a valid mode then open the fs that disk contains
    This sequence will call fat16_open function which essentially will return a 
    pointer to the its internal private data that will create for existing file.

    step 5. create a new file descriptor, set the new fs of the descriptor to the 
    file system that we opened.
    step 6. return the descriptor index.
*/
int fopen(const char* filename, const char* mode_string){
	int response = 0;
	path_root_t* root_path = pathparser_parse(filename, NULL);
	if(!root_path){
		response = -EINVARG;
		goto out;
	}
	
	if(!root_path->first){
		response = -EINVARG;
		goto out;
	}

	disk_t* disk = get_disk(root_path->drive_number);
	if(!disk){
		response = -EIO;
		goto out;
	}

	if(!disk->filesystem){
		response = -EIO;
		goto out;
	}

    FILE_MODE mode  = get_file_mode_by_string(mode_string);
    if(mode == FILE_MODE_INVALID){
        response = -EINVARG;
        goto out;
    }

    void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);
    if(ISERR(descriptor_private_data)){
        response = ERROR_I(descriptor_private_data);
        goto out;
    }

    file_descriptor_t* desc = 0;
    response = file_new_descriptor(&desc);
    if(response < 0){
        goto out;
    }
    desc->filesystem = disk->filesystem;
    desc->private = descriptor_private_data;
    desc->disk = disk;
    response = desc->index;


out:
    if(response < 0){
        response = 0;
    }

	return response;
}

int fseek(int fd, int offset, FILE_SEEK_MODE mode){
    int response = 0;
    struct file_descriptor* desc = file_get_descriptor(0);
    if(!desc){
        response = -EIO;
        goto out;
    }

    response = desc->filesystem->seek(desc->private, offset, mode);
out:
    return response;
}

int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd){
    int response = 0;
    if(size == 0 || nmemb == 0 || fd < 1){
        response = -EINVARG;
        goto out;
    }

    struct file_descriptor* desc = file_get_descriptor(fd);
    if(!desc){
        response = -EINVARG;
        goto out;
    }

    response = desc->filesystem->read(desc->disk, desc->private, size, nmemb, (char*) ptr);

out:
    return response;
}























