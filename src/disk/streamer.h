#ifndef STREAMER_H
#define STREAMER_H
#include "disk.h"

struct DiskStream
{
    int pos;
    struct Disk *disk;
};

struct DiskStream *diskstreamer_new(int);
int diskstreamer_seek(struct DiskStream *, int);
int diskstreamer_read(struct DiskStream *, void *, int);
void diskstreamer_close(struct DiskStream *);

#endif