#include "pparser.h"
#include "config.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "status.h"

static int pathparser_path_valid_format(const char * filename){
    int len = strnlen(filename,BIMBLEOS_MAX_PATH);
    return (len >= 3 && is_digit(filename[0]) && memcmp((void*)&filename[1], ":/", 2) == 0);
}


static int pathparser_get_drive_by_path(const char** path)
{
    if(!pathparser_path_valid_format(*path))
    {
        return -EBADPATH;
    }

    int drive_no = to_numeric_digit(*path[0]);

    // Add 3 bytes to skip drive number 0:/ 1:/ 2:/
    *path += 3;
    return drive_no;
}

static struct PathRoot* pathparser_create_root(int drive_number)
{
    struct PathRoot* path_r = kzalloc(sizeof(struct PathRoot));
    path_r->drive_no = drive_number;
    path_r->first = 0;
    return path_r;
}




static const char* pathparser_get_path_part(const char** path)
{
    char* result_path_part = kzalloc(BIMBLEOS_MAX_PATH);
    int i = 0;
    while(**path != '/' && **path != 0x00)
    {
        result_path_part[i] = **path;
        *path += 1;
        i++; 
    }

    if (**path == '/')
    {
        // Skip the forward slash to avoid problems
        *path += 1;
    }

    if(i == 0)
    {
        kfree(result_path_part);
        result_path_part = 0;
    }

    return result_path_part;
}


static struct PathPart* pathparser_parse_path_part(struct PathPart* last_part, const char** path)
{
    const char* path_part_str = pathparser_get_path_part(path);
    if (!path_part_str)
    {
        return 0;
    }

    struct PathPart* part = kzalloc(sizeof(struct PathPart));
    part->part = path_part_str;
    part->next = 0x00;

    if (last_part)
    {
        last_part->next = part;
    }

    return part;
}

void pathparser_free(struct PathRoot* root)
{
    struct PathPart* part = root->first;
    while(part)
    {
        struct PathPart* next_part = part->next;
        kfree((void*) part->part);
        kfree(part);
        part = next_part;
    }

    kfree(root);
}

/**
 * @brief Creates a PathRoot structure from path
 * 
 * @param path Path string 
 * @param current_directory_path 
 * @return struct PathRoot* 
 */
struct PathRoot* pathparser_parse(const char* path, const char* current_directory_path)
{
    int driveNo;
    const char* tmp_path = path;
    struct PathRoot* pathRoot = 0;

    if (strlen(path) > BIMBLEOS_MAX_PATH)
    {
        goto out;
    }

    driveNo = pathparser_get_drive_by_path(&tmp_path);
    if (driveNo < 0)
    {
        goto out;
    }

    pathRoot = pathparser_create_root(driveNo);
    if (!pathRoot)
    {
        goto out;
    }

    struct PathPart* first_part = pathparser_parse_path_part(NULL, &tmp_path);
    if (!first_part)
    {
        goto out;
    }

    pathRoot->first = first_part;
    struct PathPart* part = pathparser_parse_path_part(first_part, &tmp_path);
    while(part)
    {
        part = pathparser_parse_path_part(part, &tmp_path);
    }
    
out:
    return pathRoot;
}