#include "path_parser.h"
#include "string/string.h"
#include "memory/heap/kernel_heap.h"
#include "memory/memory.h"
#include "status.h"
#include "config.h"

/*  
    @brief 
            This function checks whether a path is valid or not
            it checks if the first element is a digit (e.g 0:/bin/...)
            then compares other 2 bytes for ":/", memcmp should return 0
            It also compares the string length of the provided path to 108 (max path length).

            Validates: 0:/ of the given path
    
    @param filename takes a constant character pointer to the filename (c style string)
    @return Returns int 1 if it's valid or 0 if not.

*/
static int pathparser_valid_format(const char* filename){
    int len = strnlen(filename, VARGOOS_MAX_PATH);

    return (len >= 3 && isdigit(filename[0]) && memcmp((void*) &filename[1], ":/", 2) == 0);
}


/*
    @brief This function extracts the first index of the path (drive numeber)
        and skips 3 bytes aka path directory (e.g for 0:/hello.txt it will skip 0:/ so it's only hello.txt
    
    @param path a double constant character pointer to the path
    @return Returns integer drive number (first element at index [0], e.g for 3:/bin, it will return 3)
*/
static int pathparser_get_drive_by_path(const char** path){
    if(!pathparser_valid_format(*path)){
        return -EBADPATH;
    }

    int drive_number = to_numeric_digit(*path[0]);
    
    //skip the path root by adding 3 bytes
    *path += 3;

    return drive_number;
}

/*
    @brief This function creates a path root. It allocates memory for path, assigns the drive number
        and sets the first (pointer to the first part to NULL) (path_r->first is the head of the parsed path).
        Path after allocation "{drive_number}/NULL".
    @param drive_number integer to the drive number (e.g 3 -> 3:/NULL after allocation)
    @return A pointer to the allocated path root
*/
static path_root_t* pathparser_create_root(int drive_number){
    path_root_t* path_r = kzalloc(sizeof(path_root_t));
    path_r->drive_number = drive_number;
    path_r->first = NULL;

    return path_r;
}

/*
    TODO: optimize the following function
*/

static const char* pathparser_get_path_part(const char** path){
    char* result_path_part = kzalloc(VARGOOS_MAX_PATH);
    int i = 0;

    while(**path != '/' && **path != 0x00){
        result_path_part[i++] = **path;
        *path += 1;
    }

    //skip the forward slash
    if(**path == '/'){
        *path += 1;
    }

    if(i == 0){
        kfree(result_path_part);
        result_path_part = NULL;
    }

    return result_path_part;
}

path_part_t* pathparser_parse_path_part(path_part_t* last_part, const char** path){

    const char* path_part_str = pathparser_get_path_part(path);

    if(!path_part_str){
        return NULL;
    }

    path_part_t* part = kzalloc(sizeof(path_part_t));
    part->part_name = path_part_str;
    part->next = NULL;

    if(last_part){
        last_part->next = part;
    }

    return part;
}

void pathparser_free(path_root_t* root){
    
    path_part_t* part = root->first;
    while(part){
        path_part_t* next_part = part->next;

        kfree((void*) part->part_name);
        kfree(part);
        part = next_part;
    }

    kfree(root);
}

path_root_t* pathparser_parse(const char* path, const char* current_directory_path){
    
    int response = 0;
    const char* temp_path = path;
    path_root_t* path_root = NULL;

    if(strlen(path) > VARGOOS_MAX_PATH){
        goto out;
    }

    response = pathparser_get_drive_by_path(&temp_path);

    if(response < 0){
        goto out;
    }

    path_root = pathparser_create_root(response);
    if(!path_root){
        goto out;
    }

    path_part_t* first_part = pathparser_parse_path_part(NULL, &temp_path);
    if(!first_part){
        goto out;
    }

    path_root->first = first_part;
    path_part_t* part = pathparser_parse_path_part(first_part, &temp_path);

    while(part){
        part = pathparser_parse_path_part(part, &temp_path);
    }

out:
    return path_root;
}
