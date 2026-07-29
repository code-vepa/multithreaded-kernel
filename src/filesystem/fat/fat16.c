#include "fat16.h"
#include "status.h"
#include "string/string.h"
#include "memory/memory.h"

int fat16_resolve(disk_t* disk);
void* fat16_open(disk_t* disk, path_part_t* path, FILE_MODE mode);

filesystem_t fat16_fs = {
    .resolve = fat16_resolve,
    .open = fat16_open
};

filesystem_t* fat16_init(){
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}


static void fat16_init_private(disk_t* disk, struct fat_private* private){
    memset(private, 0, sizeof(struct fat_private));
    private->cluster_read_stream = diskstreamer_new(disk->id);
    private->fat_read_stream = diskstreamer_new(disk->id);
    private->directory_stream = diskstreamer_new(disk->id);
}


/*
    @brief Disk searches in absolute bytes, not sector positions. So, we have to convert sectors to bytes.
*/
int fat16_sector_to_absolute(disk_t* disk, int sector){
    return sector * disk->sector_size;
}


int fat16_get_total_items_for_directory(disk_t* disk, uint32_t directory_start_sector){
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&empty_item, 0, sizeof(empty_item));

    struct fat_private* private = disk->fs_private;
    int response = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * disk->sector_size;

    disk_stream_t* stream = private->directory_stream;
    diskstreamer_seek(stream, directory_start_pos);
    while(1){
        if(diskstreamer_read(stream, &item, sizeof(item)) != STATUS_OK){
            response = -EIO;
            goto out;
        }
    }

out:
    return response;
}

/*
    @brief The equation is used to calculate the root directory position.
    That is the absolute sector position on the disk of the start of root directory.
    This function retrieves a root directory from the disk and implements its properties.
*/

int fat16_get_root_directory(disk_t* disk, struct fat_private* fat_private, struct fat_directory* directory){
    int response = 0;

    struct fat_header* primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = (root_dir_entries * sizeof(struct fat_directory_item));
    int total_sectors = root_dir_size / disk->sector_size;

    if(root_dir_size % disk->sector_size){
        ++total_sectors;
    }

    int total_items = fat16_get_total_items_for_directory(fat_private, root_dir_sector_pos);

    struct fat_directory_item* dir = kzalloc(root_dir_size);
    if(!dir){
        response = -ENOMEM;
        goto out;
    }

    struct disk_stream* stream = fat_private->directory_stream;
    if(diskstreamer_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != STATUS_OK){
        response = -EIO;
        goto out;
    }

    if(diskstreamer_read(stream, dir, root_dir_size) != STATUS_OK){
        response = -EIO;
        goto out;
    }

    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);

out:
    return response;
}


/*
    @brief If this function returns 0 then it is confirming that current filesystem can manage
    that disk because disk has this filesystem in it.
*/
int fat16_resolve(disk_t* disk){
    int response = 0;
    struct fat_private* fat_private = kzalloc(sizeof(struct fat_private)); 
    fat16_init_private(disk, fat_private);

    disk_stream_t* stream = diskstreamer_new(disk->id);
    if(!stream){
        response = -ENOMEM;
        goto out;
    }

    if(diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != STATUS_OK){
        response = -EIO;
        goto out;
    }
    
    // 0x29 is from FAT manual
    // Validate the signature from the line 42
    if(fat_private->header.shared.extended_header.signature != 0x29){
        response = -EFSNOTUS; // Not responsible
        goto out;
    }

    if(fat16_get_rppt_directory(disk, fat_private, &fat_private->root_directory) != STATUS_OK){
        response = -EIO;
        goto out;
    }


out:
    return response;
}


void* fat16_open(disk_t* disk, path_part_t* path, FILE_MODE mode){
    return 0;
}