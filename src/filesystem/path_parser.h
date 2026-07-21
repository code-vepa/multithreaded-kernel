#ifndef PATH_PARSER_H
#define PATH_PARSER_H

struct path_part{
    const char* part_name;
    struct path_part* next;
} typedef path_part_t;

struct path_root{
    int drive_number;
    path_part_t* first;
} typedef path_root_t;

#endif