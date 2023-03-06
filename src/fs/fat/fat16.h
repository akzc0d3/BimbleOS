#ifndef FAT16_H
#define FAT16_H
#include "file.h"



#define BIMBLEOS_FAT16_SIGNATURE 0x29
#define BIMBLEOS_FAT16_FAT_ENTRY_SIZE 0x02
#define BIMBLEOS_FAT16_BAD_SECTOR 0xFF7
#define BIMBLEOS_FAT16_UNUSED 0x00

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// Fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

struct FAT_Header
{
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
    uint32_t hidden_setors;
    uint32_t sectors_big;
} __attribute__((packed));

struct FAT_HeaderExtended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct FAT_H
{
    struct FAT_Header primary_header;
    union FAT_H_E
    {
        struct FAT_HeaderExtended extended_header;
    } shared;
};

struct FAT_DirectoryItem
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct FAT_Directory
{
    struct FAT_DirectoryItem *item;
    int total;
    int sector_pos;
    int ending_sector_pos;
};

struct FAT_Item
{
    union
    {
        struct FAT_DirectoryItem *item;
        struct FAT_Directory *directory;
    };

    FAT_ITEM_TYPE type;
};

struct FAT_FileDescriptor
{
    struct FAT_Item *item;
    uint32_t pos;
};

struct FAT_Private
{
    struct FAT_H header;
    struct FAT_Directory root_directory;

    // Used to stream data clusters
    struct DiskStream *cluster_read_stream;
    // Used to stream the file allocation table
    struct DiskStream *fat_read_stream;

    // Used in situations where we stream the directory
    struct DiskStream *directory_stream;
};




struct Filesystem *fat16_init();

int fat16_resolve(struct Disk *);
void *fat16_open(struct Disk *, struct PathPart *, FILE_MODE);
int fat16_read(struct Disk *disk, void *descriptor, uint32_t size, uint32_t nmemb, char *out_ptr);
int fat16_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode);
int fat16_stat(struct Disk* disk, void* private, struct FileStat* stat);
int fat16_close(void* private);


#endif