#include "file.h"
#include "config.h"
#include "status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "kernel.h"
#include "disk/disk.h"
#include "fat/fat16.h"
#include "string/string.h"


struct Filesystem* filesystems[BIMBLEOS_MAX_FILESYSTEMS];
struct FileDescriptor* file_descriptors[BIMBLEOS_MAX_FILE_DESCRIPTORS];

/**
 * @brief Return a pointer to free Filesystem from 'filesystems'
 * 
 * @return struct Filesystem** 
 */
static struct Filesystem** fs_get_free_filesystem()
{
    int i = 0;
    for (i = 0; i < BIMBLEOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0)
        {
            return &filesystems[i];
        }
    }

    return 0;
}

/**
 * @brief Insert 'filesystem' in 'filesystems'
 * 
 * @param filesystem 
 */
void fs_insert_filesystem(struct Filesystem* filesystem)
{
    struct Filesystem** fs;
    fs = fs_get_free_filesystem();
    if (!fs)
    {
        print("Problem inserting filesystem"); 
        while(1) {}
    }

    *fs = filesystem;
}

/**
 * @brief Load built-in filesystem of kernel  
 * 
 */
static void fs_static_load()
{
    fs_insert_filesystem(fat16_init());
}

/**
 * @brief Loads filesystems
 * 
 */
void fs_load()
{
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

/**
 * @brief Initialize filesystem
 * 
 */
void fs_init()
{
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static int file_new_descriptor(struct FileDescriptor** desc_out)
{
    int res = -ENOMEM;
    for (int i = 0; i < BIMBLEOS_MAX_FILE_DESCRIPTORS; i++)
    {
        if (file_descriptors[i] == 0)
        {
            struct FileDescriptor* desc = kzalloc(sizeof(struct FileDescriptor));
            // Descriptors start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }

    return res;
}

static struct FileDescriptor* file_get_descriptor(int fd)
{
    if (fd <= 0 || fd >= BIMBLEOS_MAX_FILE_DESCRIPTORS)
    {
        return 0;
    }

    // Descriptors start at 1
    int index = fd - 1;
    return file_descriptors[index];
}

struct Filesystem* fs_resolve(struct Disk* disk)
{
    struct Filesystem* fs = 0;
    for (int i = 0; i < BIMBLEOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
        {
            fs = filesystems[i];
            break;
        }
    }

    return fs;
}


int fstat(int fd, struct FileStat* stat)
{
    int res = 0;
    struct FileDescriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->stat(desc->disk, desc->private_data, stat);
out:
    return res;
}


FILE_MODE file_get_mode_by_string(const char* str)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    if (strncmp(str, "r", 1) == 0)
    {
        mode = FILE_MODE_READ;
    }
    else if(strncmp(str, "w", 1) == 0)
    {
        mode = FILE_MODE_WRITE;
    }
    else if(strncmp(str, "a", 1) == 0)
    {
        mode = FILE_MODE_APPEND;
    }
    return mode;
}



int fseek(int fd, int offset, FILE_SEEK_MODE whence)
{
    int res = 0;
    struct FileDescriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->seek(desc->private_data, offset, whence);
out:
    return res;
}
int fopen(const char* filename, const char* mode_str)
{
    int res = 0;
    struct PathRoot* root_path = pathparser_parse(filename, NULL);
    if (!root_path)
    {
        res = -EINVARG;
        goto out;
    }

    // We cannot have just a root path 0:/ 0:/test.txt
    if (!root_path->first)
    {
        res = -EINVARG;
        goto out;
    }

    // Ensure the disk we are reading from exists
    struct Disk* disk = disk_get(root_path->drive_no);
    if (!disk)
    {
        res = -EIO;
        goto out;
    }

    if (!disk->filesystem)
    {
        res = -EIO;
        goto out;
    }

    FILE_MODE mode = file_get_mode_by_string(mode_str);
    if (mode == FILE_MODE_INVALID)
    {
        res = -EINVARG;
        goto out;
    }

    void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);
    if (ISERR(descriptor_private_data))
    {
        res = ERROR_I(descriptor_private_data);
        goto out;
    }

    struct FileDescriptor* desc = 0;
    res = file_new_descriptor(&desc);
    if (res < 0)
    {
        goto out;
    }
    desc->filesystem = disk->filesystem;
    desc->private_data = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;

out:
    // fopen shouldnt return negative values
    if (res < 0)
        res = 0;

    return res;
}


static void file_free_descriptor(struct FileDescriptor* desc)
{
    file_descriptors[desc->index-1] = 0x00;
    kfree(desc);
}


int fclose(int fd)
{
    int res = 0;
    struct FileDescriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->close(desc->private_data);
    if (res == BIMBLEOS_ALL_OK)
    {
        file_free_descriptor(desc);
    }
out:
    return res;
}
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd)
{
    int res = 0;
    if (size == 0 || nmemb == 0 || fd < 1)
    {
        res = -EINVARG;
        goto out;
    }

    struct FileDescriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EINVARG;
        goto out;
    }

    res = desc->filesystem->read(desc->disk, desc->private_data, size, nmemb, (char*) ptr);
out:
    return res;
}