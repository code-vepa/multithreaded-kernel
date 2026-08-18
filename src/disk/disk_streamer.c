#include "disk_streamer.h"
#include "memory/heap/kernel_heap.h"
#include "config.h"
#include "stdbool.h"

/*
    @brief: Creates a new disk streamer and sets a byte position to 0
    @return: pointer to new created streamer (disk_stream*)
*/
disk_stream_t* diskstreamer_new(int disk_id){
    disk_t* disk = get_disk(disk_id);
    if(!disk){
        return NULL;
    }

    disk_stream_t* streamer = kzalloc(sizeof(disk_stream_t));

    streamer->pos = 0;
    streamer->disk = disk;
    return streamer;
}

/*
    @brief: Changes the position of the disk streamer to provided position
    @return: int 0
*/
int diskstreamer_seek(disk_stream_t* streamer, int pos){
    streamer->pos = pos;
    return 0;
}

/*
    @brief This reads a sector into memory and offsets to the correct position.
*/
int diskstreamer_read(disk_stream_t* stream, void* out, int total){
    int sector = stream->pos / VARGOOS_SECTOR_SIZE;
    int offset = stream->pos % VARGOOS_SECTOR_SIZE;
    int total_to_read = total;
    bool overflow = (offset + total_to_read) >= VARGOOS_SECTOR_SIZE;
    char buffer[VARGOOS_SECTOR_SIZE];

    if(overflow){
        total_to_read -= (offset + total_to_read) - VARGOOS_SECTOR_SIZE;
    }

    int response = disk_read_block(stream->disk, sector, 1, buffer);

    if(response < 0){
        goto out;
    }
    
    for(int i = 0; i < total_to_read; ++i){
        *(char*)out++ = buffer[offset + i];
    }

    stream->pos += total_to_read;
    if(overflow){
        response = diskstreamer_read(stream, out, total - total_to_read);
    }

out:
    return response;
}

void diskstreamer_close(disk_stream_t* stream){
    kfree(stream);
};

