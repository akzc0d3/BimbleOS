#include "fat16.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "status.h"
#include "config.h"
#include "fs/file.h"
#include "memory/memory.h"
#include "kernel.h"
#include "memory/heap/kheap.h"


struct Filesystem fat16_fs = {
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .close = fat16_close,
};

struct Filesystem * fat16_init()
{
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}


int fat16_get_total_items_for_directory(struct Disk *disk, uint32_t directory_start_sector)
{
    struct FAT_DirectoryItem item;
    struct FAT_DirectoryItem empty_item;
    memset(&empty_item, 0, sizeof(empty_item));

    struct FAT_Private *fat_private = disk->fs_private;

    int res = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * disk->sector_size;
    struct DiskStream *stream = fat_private->directory_stream;
    if (diskstreamer_seek(stream, directory_start_pos) != BIMBLEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    while (1)
    {
        if (diskstreamer_read(stream, &item, sizeof(item)) != BIMBLEOS_ALL_OK)
        {
            res = -EIO;
            goto out;
        }

        if (item.filename[0] == 0x00)
        {
            // We are done
            break;
        }

        // Is the item unused
        if (item.filename[0] == 0xE5)
        {
            continue;
        }

        i++;
    }

    res = i;

out:
    return res;
}

int fat16_sector_to_absolute(struct Disk *disk, int sector)
{
    return sector * disk->sector_size;
}

/**
 * @brief Extract info about root directory from FAT header.
 *        Initialize @param directory with that info.
 * 
 * @param disk 
 * @param fat_private 
 * @param directory 
 * @return int 
 */
int fat16_get_root_directory(struct Disk *disk, struct FAT_Private *fat_private, struct FAT_Directory *directory)
{
    int res = 0;
    struct FAT_DirectoryItem *dir = 0x00;

    // Read info about root directory form FAT header
    struct FAT_Header *primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = (root_dir_entries * sizeof(struct FAT_DirectoryItem));
    int total_sectors = root_dir_size / disk->sector_size;


    if (root_dir_size % disk->sector_size)
    {
        total_sectors += 1;
    }

    
    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);

    dir = kzalloc(root_dir_size);
    if (!dir)
    {
        res = -ENOMEM;
        goto err_out;
    }

    struct DiskStream *stream = fat_private->directory_stream;
    if (diskstreamer_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != BIMBLEOS_ALL_OK)
    {
        res = -EIO;
        goto err_out;
    }

    if (diskstreamer_read(stream, dir, root_dir_size) != BIMBLEOS_ALL_OK)
    {
        res = -EIO;
        goto err_out;
    }

    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);
    
    out:
    return res;

    err_out:
    if (dir)
    {
        kfree(dir);
    }

    return res;
}

static void fat16_init_private(struct Disk *disk, struct FAT_Private *private)
{
    memset(private, 0, sizeof(struct FAT_Private));
    private->cluster_read_stream = diskstreamer_new(disk->id);
    private->fat_read_stream = diskstreamer_new(disk->id);
    private->directory_stream = diskstreamer_new(disk->id);
}

/**
 * @brief Return zero if the disk has FAT16 filesystem
 * 
 * @param disk 
 * @return int 0 : fat16_resolve return 0 if the disk have FAT16 filesystem
 *
 */
int fat16_resolve(struct Disk *disk)
{
    int res = 0;
    struct FAT_Private *fat_private = kzalloc(sizeof(struct FAT_Private));
    fat16_init_private(disk, fat_private);

    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;

    struct DiskStream *stream = diskstreamer_new(disk->id);
    if (!stream)
    {
        res = -ENOMEM;
        goto out;
    }


    // Try to read the FAT16 header in boot sector
    if (diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != BIMBLEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    // Check if the signature matches
    if (fat_private->header.shared.extended_header.signature != 0x29)
    {
        res = -EFSNOTUS;
        goto out;
    }


    if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != BIMBLEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

out:
    if (stream)
    {
        diskstreamer_close(stream);
    }

    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    return res;
}

void fat16_to_proper_string(char **out, const char *in, size_t size)
{
    int i = 0;
    while (*in != 0x00 && *in != 0x20)
    {
        **out = *in;
        *out += 1;
        in += 1;
        // We cant process anymore since we have exceeded the input buffer size
        if (i >= size-1)
        {
            break;
        }
        i++;
    }

    **out = 0x00;
}

void fat16_get_full_relative_filename(struct FAT_DirectoryItem *item, char *out, int max_len)
{
    memset(out, 0x00, max_len);
    char *out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char *)item->filename, sizeof(item->filename));
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20)
    {
        *out_tmp++ = '.';
        fat16_to_proper_string(&out_tmp, (const char *)item->ext, sizeof(item->ext));
    }
}

struct FAT_DirectoryItem *fat16_clone_directory_item(struct FAT_DirectoryItem *item, int size)
{
    struct FAT_DirectoryItem *item_copy = 0;
    if (size < sizeof(struct FAT_DirectoryItem))
    {
        return 0;
    }

    item_copy = kzalloc(size);
    if (!item_copy)
    {
        return 0;
    }

    memcpy(item_copy, item, size);
    return item_copy;
}

static uint32_t fat16_get_first_cluster(struct FAT_DirectoryItem *item)
{
    return (item->high_16_bits_first_cluster) | item->low_16_bits_first_cluster;
};

static int fat16_cluster_to_sector(struct FAT_Private *private, int cluster)
{
    return private->root_directory.ending_sector_pos + ((cluster - 2) * private->header.primary_header.sectors_per_cluster);
}

static uint32_t fat16_get_first_fat_sector(struct FAT_Private *private)
{
    return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(struct Disk *disk, int cluster)
{
    int res = -1;
    struct FAT_Private *private = disk->fs_private;
    struct DiskStream *stream = private->fat_read_stream;
    if (!stream)
    {
        goto out;
    }

    uint32_t fat_table_position = fat16_get_first_fat_sector(private) * disk->sector_size;
    res = diskstreamer_seek(stream, fat_table_position * (cluster * BIMBLEOS_FAT16_FAT_ENTRY_SIZE));
    if (res < 0)
    {
        goto out;
    }

    uint16_t result = 0;
    res = diskstreamer_read(stream, &result, sizeof(result));
    if (res < 0)
    {
        goto out;
    }

    res = result;
out:
    return res;
}

/**
 * @brief Gets the correct cluster to use based on the starting cluster and the offset
 * 
 * @param disk 
 * @param starting_cluster 
 * @param offset 
 * @return int 
 */
static int fat16_get_cluster_for_offset(struct Disk *disk, int starting_cluster, int offset)
{
    int res = 0;
    struct FAT_Private *private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;
    for (int i = 0; i < clusters_ahead; i++)
    {
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        if (entry == 0xFF8 || entry == 0xFFF)
        {
            // We are at the last entry in the file
            res = -EIO;
            goto out;
        }

        // Sector is marked as bad?
        if (entry == BIMBLEOS_FAT16_BAD_SECTOR)
        {
            res = -EIO;
            goto out;
        }

        // Reserved sector?
        if (entry == 0xFF0 || entry == 0xFF6)
        {
            res = -EIO;
            goto out;
        }

        if (entry == 0x00)
        {
            res = -EIO;
            goto out;
        }

        cluster_to_use = entry;
    }

    res = cluster_to_use;
out:
    return res;
}
static int fat16_read_internal_from_stream(struct Disk *disk, struct DiskStream *stream, int cluster, int offset, int total, void *out)
{
    int res = 0;
    struct FAT_Private *private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    if (cluster_to_use < 0)
    {
        res = cluster_to_use;
        goto out;
    }

    int offset_from_cluster = offset % size_of_cluster_bytes;

    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = diskstreamer_seek(stream, starting_pos);
    if (res != BIMBLEOS_ALL_OK)
    {
        goto out;
    }

    res = diskstreamer_read(stream, out, total_to_read);
    if (res != BIMBLEOS_ALL_OK)
    {
        goto out;
    }

    total -= total_to_read;
    if (total > 0)
    {
        // We still have more to read
        res = fat16_read_internal_from_stream(disk, stream, cluster, offset + total_to_read, total, out + total_to_read);
    }

out:
    return res;
}

static int fat16_read_internal(struct Disk *disk, int starting_cluster, int offset, int total, void *out)
{
    struct FAT_Private *fs_private = disk->fs_private;
    struct DiskStream *stream = fs_private->cluster_read_stream;
    return fat16_read_internal_from_stream(disk, stream, starting_cluster, offset, total, out);
}

void fat16_free_directory(struct FAT_Directory *directory)
{
    if (!directory)
    {
        return;
    }

    if (directory->item)
    {
        kfree(directory->item);
    }

    kfree(directory);
}

void fat16_fat_item_free(struct FAT_Item *item)
{
    if (item->type == FAT_ITEM_TYPE_DIRECTORY)
    {
        fat16_free_directory(item->directory);
    }
    else if (item->type == FAT_ITEM_TYPE_FILE)
    {
        kfree(item->item);
    }

    kfree(item);
}

struct FAT_Directory *fat16_load_fat_directory(struct Disk *disk, struct FAT_DirectoryItem *item)
{
    int res = 0;
    struct FAT_Directory *directory = 0;
    struct FAT_Private *fat_private = disk->fs_private;
    if (!(item->attribute & FAT_FILE_SUBDIRECTORY))
    {
        res = -EINVARG;
        goto out;
    }

    directory = kzalloc(sizeof(struct FAT_Directory));
    if (!directory)
    {
        res = -ENOMEM;
        goto out;
    }

    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
    int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
    directory->total = total_items;
    int directory_size = directory->total * sizeof(struct FAT_DirectoryItem);
    directory->item = kzalloc(directory_size);
    if (!directory->item)
    {
        res = -ENOMEM;
        goto out;
    }

    res = fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item);
    if (res != BIMBLEOS_ALL_OK)
    {
        goto out;
    }

out:
    if (res != BIMBLEOS_ALL_OK)
    {
        fat16_free_directory(directory);
    }
    return directory;
}
struct FAT_Item *fat16_new_fat_item_for_directory_item(struct Disk *disk, struct FAT_DirectoryItem *item)
{
    struct FAT_Item *f_item = kzalloc(sizeof(struct FAT_Item));
    if (!f_item)
    {
        return 0;
    }

    if (item->attribute & FAT_FILE_SUBDIRECTORY)
    {
        f_item->directory = fat16_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
        return f_item;
    }

    f_item->type = FAT_ITEM_TYPE_FILE;
    f_item->item = fat16_clone_directory_item(item, sizeof(struct FAT_DirectoryItem));
    return f_item;
}

struct FAT_Item *fat16_find_item_in_directory(struct Disk *disk, struct FAT_Directory *directory, const char *name)
{
    struct FAT_Item *f_item = 0;
    char tmp_filename[BIMBLEOS_MAX_PATH];
    for (int i = 0; i < directory->total; i++)
    {
        fat16_get_full_relative_filename(&directory->item[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0)
        {
            // Found it let's create a new fat_item
            f_item = fat16_new_fat_item_for_directory_item(disk, &directory->item[i]);
        }
    }

    return f_item;
}

struct FAT_Item *fat16_get_directory_entry(struct Disk *disk, struct PathPart* path)
{
    struct FAT_Private *fat_private = disk->fs_private;
    struct FAT_Item *current_item = 0;
    struct FAT_Item *root_item = fat16_find_item_in_directory(disk, &fat_private->root_directory, path->part);
    if (!root_item)
    {
        goto out;
    }

    struct PathPart *next_part = path->next;
    current_item = root_item;
    while (next_part != 0)
    {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY)
        {
            current_item = 0;
            break;
        }

        struct FAT_Item *tmp_item = fat16_find_item_in_directory(disk, current_item->directory, next_part->part);
        fat16_fat_item_free(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }
out:
    return current_item;
}

/**
 * @brief 
 * 
 * @param disk 
 * @param path_part 
 * @param filemode 
 * @return void* Pointer to file descriptor
 */
void *fat16_open(struct Disk *disk, struct PathPart *path_part, FILE_MODE filemode)
{
   struct FAT_FileDescriptor *descriptor = 0;
    int err_code = 0;
    // Currently only support reading     
    if (filemode != FILE_MODE_READ)
    {
        err_code = -ERDONLY;
        goto err_out;
    }

    descriptor = kzalloc(sizeof(struct FAT_FileDescriptor));
    if (!descriptor)
    {
        err_code = -ENOMEM;
        goto err_out;
    }

    descriptor->item = fat16_get_directory_entry(disk, path_part);
    if (!descriptor->item)
    {
        err_code = -EIO;
        goto err_out;
    }

    descriptor->pos = 0;
    return descriptor;

err_out:
    if(descriptor)
        kfree(descriptor);

    return ERROR(err_code);
}

static void fat16_free_file_descriptor(struct FAT_FileDescriptor* desc)
{
    fat16_fat_item_free(desc->item);
    kfree(desc);
}
int fat16_close(void* private)
{
    fat16_free_file_descriptor((struct FAT_FileDescriptor*) private);
    return 0;
}

int fat16_stat(struct Disk* Disk, void* private, struct FileStat* stat)
{
    int res = 0;
    struct FAT_FileDescriptor* descriptor = (struct FAT_FileDescriptor*) private;
    struct FAT_Item* desc_item = descriptor->item;
    if (desc_item->type != FAT_ITEM_TYPE_FILE)
    {
        res = -EINVARG;
        goto out;
    }

    struct FAT_DirectoryItem* ritem = desc_item->item;
    stat->filesize = ritem->filesize;
    stat->flags = 0x00;

    if (ritem->attribute & FAT_FILE_READ_ONLY)
    {
        stat->flags |= FILE_STAT_READ_ONLY;
    }
out:
    return res;
}

int fat16_read(struct Disk *disk, void *descriptor, uint32_t size, uint32_t nmemb, char *out_ptr)
{
    int res = 0;
    struct FAT_FileDescriptor *fat_desc = descriptor;
    struct FAT_DirectoryItem *item = fat_desc->item->item;
    int offset = fat_desc->pos;
    for (uint32_t i = 0; i < nmemb; i++)
    {
        res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out_ptr);
        if (ISERR(res))
        {
            goto out;
        }

        out_ptr += size;
        offset += size;
    }

    res = nmemb;
out:
    return res;
}

int fat16_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode)
{
    int res = 0;
    struct FAT_FileDescriptor *desc = private;
    struct FAT_Item *desc_item = desc->item;
    if (desc_item->type != FAT_ITEM_TYPE_FILE)
    {
        res = -EINVARG;
        goto out;
    }

    struct FAT_DirectoryItem *ritem = desc_item->item;
    if (offset >= ritem->filesize)
    {
        res = -EIO;
        goto out;
    }

    switch (seek_mode)
    {
    case SEEK_SET:
        desc->pos = offset;
        break;

    case SEEK_END:
        res = -EUNIMP;
        break;

    case SEEK_CUR:
        desc->pos += offset;
        break;

    default:
        res = -EINVARG;
        break;
    }
out:
    return res;
}



