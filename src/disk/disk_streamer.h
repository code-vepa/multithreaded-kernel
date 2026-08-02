#ifndef DISK_STREAMER_H
#define DISK_STREAMER_H

#include "disk.h"

typedef struct disk_stream{
    int pos; //byte position
    disk_t* disk;
} disk_stream_t;

disk_stream_t* diskstreamer_new(int disk_id);
int diskstreamer_seek(disk_stream_t* streamer, int pos);
int diskstreamer_read(disk_stream_t* stream, void* out, int total);
void diskstreamer_close(disk_stream_t* stream);

#endif