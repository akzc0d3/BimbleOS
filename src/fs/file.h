#ifndef FILE_H
#define FILE_H
#include "pparser.h"
#include "pparser.h"
#include <stdint.h>

typedef unsigned int FILE_SEEK_MODE;
enum
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;
typedef unsigned int FILE_STAT_FLAGS;

enum
{
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

enum
{
    FILE_STAT_READ_ONLY = 0b00000001
};

struct Disk;
// Each FileSystem have its own implementation of function that is then pointed by below file pointer
typedef void *(*FS_OPEN_FUNCTION)(struct Disk *, struct PathPart *, FILE_MODE);
typedef int (*FS_READ_FUNCTION)(struct Disk *disk, void *private, uint32_t size, uint32_t nmemb, char *out);
typedef int (*FS_RESOLVE_FUNCTION)(struct Disk *);
typedef int (*FS_CLOSE_FUNCTION)(void *);
typedef int (*FS_SEEK_FUNCTION)(void *, uint32_t, FILE_SEEK_MODE);

struct FileStat
{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
};

typedef int (*FS_STAT_FUNCTION)(struct Disk* disk, void* private, struct FileStat* stat);



struct Filesystem
{
    // Filesystem should return zero from resolve if the provided disk is using its filesystem
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_CLOSE_FUNCTION close;
    FS_STAT_FUNCTION stat;
    char name[20];
};

struct FileDescriptor
{
    // The descriptor index
    int index;
    struct Filesystem *filesystem;

    // Private data for internal file descriptor
    void *private_data;

    // The disk that the file descriptor should be used on
    struct Disk *disk;
};



void fs_init();
int fopen(const char *, const char *);
int fseek(int, int, FILE_SEEK_MODE);
int fread(void *, uint32_t , uint32_t, int);
int fstat(int, struct FileStat *);
int fclose(int);
void fs_insert_filesystem(struct Filesystem *);
struct Filesystem *fs_resolve(struct Disk *);

#endif