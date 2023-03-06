#ifndef DISK_H
#define DISK_H

typedef unsigned int PEACHOS_DISK_TYPE;

// Represent real physical hard disk
#define BIMBLEOS_DISK_TYPE_REAL 0

struct Disk
{
    PEACHOS_DISK_TYPE type;
    int id; 
    int sector_size;
    struct Filesystem* filesystem;

    void* fs_private;
};

void disk_search_and_init();
struct Disk * disk_get(int);
int disk_read_block(struct Disk *, int, int, void *);

#endif