#ifndef FAT16_H
#define FAT16_H


#include <stdint.h>
#include "filesystem/file.h"
#include "disk/disk.h"
#include "disk/disk_streamer.h"

#define VARGOOS_FAT16_SIGNATURE 0x29
#define VARGOOS_FAT16_FAT_ENTRY_SIZE 0x02
#define VARGOOS_FAT16_BAD_SECTOR 0XFF7
#define VARGOOS_FAT16_UNUSED 0X00

/*  
    These are used for the representation of directories and file
    They shall not be stored on the disk.
*/

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// FAT directory entry attributes bitmask
#define FAT_FILE_READ_ONLY      0x01
#define FAT_FILE_HIDDEN         0X02
#define FAT_FILE_SYSTEM         0X04
#define FAT_FILE_VOLUME_LABEL   0X08
#define FAT_FILE_SUBDIRECTORY   0X10
#define FAT_FILE_ARCHIVED       0X20
#define FAT_FILE_DEVICE         0x40
#define FAT_FILE_RESERVED       0X80

// Extended BIOS Parameter Block (Dos 4.0)
struct fat_header_extended{
    uint8_t drive_no;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

// FAT16 header
struct fat_header{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));


struct fat_h{
    struct fat_header primary_header;

    // Fat header extended for future extensions
    union fat_h_e{
        struct fat_header_extended extended_header;
    } shared;
};

struct fat_directory_item{
    uint8_t filename[8]; // 8 bytes for the filename
    uint8_t ext[3]; // 3 bytes for an extension
    uint8_t attribute; // Represents attribute bitmasks
    uint8_t reserved; // Reserved for future purposes
    uint8_t creation_time_tenth_of_sec; 
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time; // Last modification time
    uint16_t last_mod_date; // & date
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory{
    struct fat_directory_item* item;
    int total; // Total number of items in fat directory
    int sector_pos; // First sector where fat directory is
    int ending_sector_pos; // Last sector of where fat directory is storing other dir and files
};

/*
    If this FAT item represents a file, then we will access item variable.
    Otherwise, we will access a directory variable.
*/
struct fat_item{
    union 
    {
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };
    
    FAT_ITEM_TYPE type;
};


struct fat_item_descriptor{
    struct fat_item* item;
    uint32_t pos;
};

struct fat_private{
    struct fat_h header;
    struct fat_directory root_directory;

    // Used to stream data clusters
    struct disk_stream* cluster_read_stream;

    //Used to stream the FAT
    struct disk_stream* fat_read_stream;

    //Used to stream directories
    struct disk_stream* directory_stream;
};

filesystem_t* fat16_init();
#endif