#include "fat16.h"
#include "status.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/heap/kernel_heap.h"
#include "config.h"
#include "kernel.h"

int fat16_resolve(disk_t* disk);
void* fat16_open(disk_t* disk, path_part_t* path, FILE_MODE mode);
int fat16_read(disk_t* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out);
int fat16_seek(void* private, uint32_t offset, FILE_SEEK_MODE mode);
int fat16_stat(disk_t* disk, void* private, struct file_stat* stat);
int fat16_close(void* private);

filesystem_t fat16_fs = {
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .close = fat16_close
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
    if(diskstreamer_seek(stream, directory_start_pos) != STATUS_OK){
		response = -EIO;
		goto out;
	}

	while(1){
        if(diskstreamer_read(stream, &item, sizeof(item)) != STATUS_OK){
            response = -EIO;
            goto out;
        }
    	
		if(item.filename[0] == 0x00){
			break;
		}

		if(item.filename[0] == 0xE5){
			continue;
		}
		++i; // Increase a total count
	}
	response = i;

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

    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);

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
	
	disk->fs_private = fat_private;
	disk->filesystem = &fat16_fs;
    
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

    if(fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != STATUS_OK){
        response = -EIO;
        goto out;
    }
	


out:
	if(stream){
		diskstreamer_close(stream);
	}

	if(response < 0){
		kfree(fat_private);
		disk->fs_private = 0;
	}
    
	return response;
}

/*
    @brief This function goes through string and will look
    for space or null terminator, once found it will null terminate
    that space.
*/
void fat16_to_proper_string(char** out, char* in){
    while(*in != 0x00 && *in != 0x20){
        **out = *in;
        ++*out, ++in;
    }

    if(*in == 0x20){
        **out = 0x00;
    }
}

/*
    @brief This function will convert a file name to the proper
    string format and add an extention at the end.

    e.g: filename: test    ext: txt   -> test.txt (will get rid of all spaces at the end)
*/
void fat16_get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len){
    memset(out, 0, max_len);
    char* out_temp = out;
    fat16_to_proper_string(&out_temp, (char*) item->filename);
    if(item->ext[0] != 0x00 && item->ext[0] != 0x20){
        *out_temp++ = '.';
        fat16_to_proper_string(&out_temp, (char*) item->ext);
    }
}

struct fat_directory_item* fat16_clone_directory_item(struct fat_directory_item* item, int size){
    struct fat_directory_item* item_copy = 0;
    if(size < sizeof(struct fat_directory_item)){
        return 0;
    }
    
    item_copy = kzalloc(size);
    if(!item_copy){
        return 0;
    }
    memcpy(item_copy, item, size);
    
    return item_copy;
}

static uint32_t fat16_get_first_cluster(struct fat_directory_item* item){
    return item->high_16_bits_first_cluster | item->low_16_bits_first_cluster;
}

/*
    @brief Takes an ending position of the root directory and adds our cluster number subtracted by 2
            multiplied by the sectors per cluster which will get us the sector that passed cluster 
            represents
*/
static int fat16_cluster_to_sector(struct fat_private* private, int cluster){
    return private->root_directory.ending_sector_pos + ((cluster - 2) 
        * private->header.primary_header.sectors_per_cluster);
}

static uint32_t fat16_get_first_fat_sector(struct fat_private* private){
    return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(disk_t* disk, int cluster){
    int response = -1;

    struct fat_private* private = disk->fs_private;
    disk_stream_t* stream = private->fat_read_stream;
    if(!stream){
        goto out;
    }

    uint32_t fat_table_position = fat16_get_first_fat_sector(private) * disk->sector_size;
    response = diskstreamer_seek(stream, fat_table_position * (cluster * VARGOOS_FAT16_FAT_ENTRY_SIZE));
    if(response < 0){
        goto out;
    }

    uint16_t result = 0;
    response = diskstreamer_read(stream, &result, sizeof(result));
    if(response < 0){
        goto out;
    }
    
    response = result;
out:
    return response;
}

// Returns the correct cluster
static int fat16_get_cluster_for_offset(disk_t* disk, int cluster, int offset){
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;

    for(int i = 0; i < clusters_ahead; ++i){
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        if(0xFF8 == entry || 0xFFF == entry){
            // Last entry of the file 
            res = -EIO;
            goto out;
        }

        if(entry == VARGOOS_FAT16_BAD_SECTOR){
            res = -EIO;
            goto out;
        }

        // Reserved
        if(0xFF0 == entry || 0xFF6 == entry){
            res = -EIO;
            goto out;
        }

        if(entry == 0x00){
            res = -EIO;
            goto out;
        }

        cluster_to_use = entry;
    }

    res = cluster_to_use;
out:
    return res;
}

static int fat16_read_internal_from_stream(disk_t* disk, disk_stream_t* stream, 
                            int cluster, int offset, int total, void* out)
{    
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    
    if(cluster_to_use < 0){
        res = cluster_to_use;
        goto out;
    }

    int offset_from_cluster = offset % size_of_cluster_bytes;
    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = diskstreamer_seek(stream, starting_pos);
    if(res != STATUS_OK){
        goto out;
    }

    res = diskstreamer_read(stream, out, total_to_read);
    if(res != STATUS_OK){
        goto out;
    }

    total -= total_to_read;
    if(total > 0){
        // more to read
        res = fat16_read_internal_from_stream(disk, stream, cluster,
                offset + total_to_read, total, out + total_to_read);
    }

out:
    return res;
}

static int fat16_read_internal(disk_t* disk, int start, int offset, int total, void* out){
    struct fat_private* fs_private = disk->fs_private;
    struct disk_stream* stream = fs_private->cluster_read_stream;
    return fat16_read_internal_from_stream(disk, stream, start, offset, total, out);
}

void fat16_free_directory(struct fat_directory* directory){
    if(!directory)
        return;
    
    if(directory->item){
        kfree(directory->item);
    }

    kfree(directory);
}

void fat16_fat_item_free(struct fat_item* item){
    if(item->type == FAT_ITEM_TYPE_DIRECTORY){
        fat16_free_directory(item->directory);
    }

    else if(item->type == FAT_ITEM_TYPE_FILE){
        kfree(item->item);
    }
    
    kfree(item);
}

struct fat_directory* fat16_load_fat_directory(disk_t* disk, struct fat_directory_item* item){
    int res = 0;
    struct fat_directory* directory = 0;
    struct fat_private* fat_private = disk->fs_private;
    // Check for it to be a directory and not file
    if(!(item->attribute & FAT_FILE_SUBDIRECTORY)){
        res = -EINVARG;
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));
    if(!directory){
        res = -ENOMEM;
        goto out;
    }

    // Calculate a cluster
    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
    int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
    directory->total = total_items;
    int directory_size = directory->total * sizeof(struct fat_directory_item);
    directory->item = kzalloc(directory_size);
    if(!directory->item){
        res = -ENOMEM;
        goto out;
    }

    res = fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item);
    if(res != STATUS_OK){
        goto out;
    }

out:
    if(res != STATUS_OK){
        fat16_free_directory(directory);
    }

    return directory;
}

struct fat_item* fat16_new_fat_item_for_directory(disk_t* disk, struct fat_directory_item* item){
    struct fat_item* fat_item = kzalloc(sizeof(struct fat_item));
    if(!fat_item)
        return 0;
    
    if(item->attribute & FAT_FILE_SUBDIRECTORY){
        fat_item->directory = fat16_load_fat_directory(disk, item);
        fat_item->type = FAT_ITEM_TYPE_DIRECTORY;
    }

    fat_item->type = FAT_ITEM_TYPE_FILE;
    fat_item->item = fat16_clone_directory_item(item, sizeof(struct fat_directory_item));
    return fat_item;
}

struct fat_item* fat16_find_item_in_directory(disk_t* disk, 
                                struct fat_directory* directory, 
                                const char* part){
    struct fat_item* fat_item = 0;
    char temp_filename[VARGOOS_MAX_PATH];

    for(int i = 0; i < directory->total; ++i){
        fat16_get_full_relative_filename(&directory->item[i], temp_filename, sizeof(temp_filename));
        if(istrncmp(temp_filename, part, sizeof(temp_filename)) == 0){
            fat_item = fat16_new_fat_item_for_directory(disk, &directory->item[i]);
        }
    }

    return fat_item;
}

struct fat_item* fat16_get_directory_entry(disk_t* disk, path_part_t* path){
    struct fat_private* fat_private = disk->fs_private;
    struct fat_item* current_item = 0;
    struct fat_item* root_item = fat16_find_item_in_directory(disk, 
                                                &fat_private->root_directory,
                                                path->part_name);
    if(!root_item){
        goto out;
    }

    path_part_t* next_part = path->next;
    current_item = root_item;
    while(next_part != 0){
        if(current_item->type != FAT_ITEM_TYPE_DIRECTORY){
            current_item = 0;
            break;
        }

        struct fat_item* temp_item = fat16_find_item_in_directory(disk, current_item->directory, next_part->part_name);
        fat16_fat_item_free(current_item);
        current_item = temp_item;
        next_part = next_part->next;
    }

out:
    return current_item;
}

void* fat16_open(disk_t* disk, path_part_t* path, FILE_MODE mode){
    if(mode != FILE_MODE_READ){
        return ERROR(-ERDONLY);
    }

    struct fat_file_descriptor* descriptor = 0;
    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if(!descriptor){
        return ERROR(-ENOMEM);
    }

    descriptor->item = fat16_get_directory_entry(disk, path);
    if(!descriptor->item){
        return ERROR(-EIO);   
    }

    descriptor->pos = 0;
    return descriptor;
}

/*
    @return The amount of nmemb's that were read.
*/
int fat16_read(disk_t* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out){
    int response = 0;
    struct fat_file_descriptor* fat_desc = descriptor;
    struct fat_directory_item* item = fat_desc->item->item;

    int offset = fat_desc->pos;

    for(uint32_t i = 0; i < nmemb; ++i){
        response = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out);
        if(ISERR(response)){
            goto out;
        }
        
        out += size;
        offset += size;
    }

    response = nmemb;
out:
    return response;
}

int fat16_seek(void* private, uint32_t offset, FILE_SEEK_MODE mode){
    int response = 0;
    struct fat_file_descriptor* desc = private;
    struct fat_item* desc_item = desc->item;
    if(desc_item->type != FAT_ITEM_TYPE_FILE){
        response = -EINVARG;
        goto out;
    }

    struct fat_directory_item* dir_item = desc_item->item;
    if(offset >= dir_item->filesize){
        response = -EIO;
        goto out;
    }

    switch(mode){
        case SEEK_SET:
            desc->pos = offset;
            break;
        
        case SEEK_END:
            response = -EUNIMP;
            break;
        
        case SEEK_CURRENT:
            desc->pos += offset;
            break;
        
        default:
            response = -EINVARG;
            break;

    }

out:
    return response;
}

int fat16_stat(disk_t* disk, void* private, struct file_stat* stat){
    int response = 0;

    struct fat_file_descriptor* desc = (struct fat_file_descriptor*) private;
    struct fat_item* desc_item = desc->item;
    if(desc_item->type != FAT_ITEM_TYPE_FILE){
        response = -EINVARG;
        goto out;
    } 

    struct fat_directory_item* dir_item = desc_item->item;
    stat->filesize = dir_item->filesize;
    stat->flags = 0x00;
    
    if(dir_item->attribute & FAT_FILE_READ_ONLY){
        stat->flags |= FILE_STAT_READ_ONLY;
    }

out:
    return response;
}

static void fat16_free_file_descriptor(struct fat_file_descriptor* desc){
    fat16_fat_item_free(desc->item);
    kfree(desc);
}

int fat16_close(void* private){
    
    fat16_free_file_descriptor((struct fat_file_descriptor*) private);
    return 0;
}
