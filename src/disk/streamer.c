#include "streamer.h"
#include "memory/heap/kheap.h"
#include "config.h"
#include <stdbool.h>


/**
 * @brief Return struct DiskStream from given disk ID
 * 
 * @param disk_id 
 * @return struct DiskStream* 
 */
struct DiskStream * diskstreamer_new(int disk_id)
{


    struct Disk* disk = disk_get(disk_id);

    if(!disk){
        return 0;
    }

    struct DiskStream* diskStreamer = kzalloc(sizeof(diskStreamer));
    diskStreamer->pos = 0;
    diskStreamer->disk = disk;
    return diskStreamer;

}


int diskstreamer_seek(struct DiskStream* stream, int pos)
{
    stream->pos = pos;
    return 0;
}


/**
 * @brief Read 'total' bytes into 'out'
 * 
 * @param stream 
 * @param out 
 * @param total 
 * @return int 
 */
int diskstreamer_read(struct DiskStream* stream, void* out, int total)
{
    int sector = stream->pos / BIMBLEOS_SECTOR_SIZE;
    int offset = stream->pos % BIMBLEOS_SECTOR_SIZE;
    int total_to_read = total;
    bool overflow = (offset+total_to_read) >= BIMBLEOS_SECTOR_SIZE;
    char buff[BIMBLEOS_SECTOR_SIZE];

    if (overflow)
    {
        total_to_read -= (offset+total_to_read) - BIMBLEOS_SECTOR_SIZE;
    }

    int res = disk_read_block(stream->disk, sector, 1, buff);
    if (res < 0)
    {
        goto out;
    }

   
    for (int i = 0; i < total_to_read; i++)
    {
        *(char*)out++ = buff[offset+i];
    }

    // Adjust the stream
    stream->pos += total_to_read;
    if (overflow)
    {
        res = diskstreamer_read(stream, out, total-total_to_read);
    }
out:
    return res;
}


void diskstreamer_close(struct DiskStream* stream)
{
    kfree(stream);
}














