#ifndef PATH_PARSER_H
#define PATH_PARSER_H

typedef struct path_part{
    const char* part_name;
    struct path_part* next;
} path_part_t;

typedef struct path_root{
    int drive_number;
    path_part_t* first;
} path_root_t;


path_root_t* pathparser_parse(const char* path, const char* current_directory_path);
void pathparser_free(path_root_t* root);

#endif