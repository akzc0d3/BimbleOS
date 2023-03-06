#ifndef PARSER_H
#define PARSER_H

struct PathRoot
{
    int drive_no;
    struct PathPart *first;
};

struct PathPart
{
    const char *part;
    struct PathPart *next;
};

void pathparser_free(struct PathRoot *);
struct PathRoot *pathparser_parse(const char *, const char *);

#endif