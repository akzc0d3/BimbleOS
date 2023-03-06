#ifndef STRING_H
#define STRING_H
#include <stdbool.h>
#include <stddef.h>

size_t strlen(const char *);
size_t strnlen(const char *, int);
bool is_digit(char);
int to_numeric_digit(char);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, int);
int strnlen_terminator(const char *, int, char);
int strncmp(const char *, const char *, int);
int istrncmp(const char *, const char *, int);

#endif