#include "string.h"

char tolower(char s1)
{
    if (s1 >= 65 && s1 <= 90)
    {
        s1 += 32;
    }

    return s1;
}

/**
 * @brief Get size of string
 *
 * @param str
 * @return size_t
 */
size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}

size_t strnlen(const char *str, int max)
{
    size_t len = 0;
    for (len = 0; len < max; len++)
    {
        if (str[len] == 0)
            break;
        ;
    }

    return len;
}

bool is_digit(char c)
{
    return c >= 48 && c <= 57;
}

int to_numeric_digit(char c)
{
    return c - 48;
}

int istrncmp(const char *s1, const char *s2, int n)
{
    unsigned char u1, u2;
    while (n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

int strnlen_terminator(const char *str, int max, char terminator)
{
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (str[i] == '\0' || str[i] == terminator)
            break;
    }

    return i;
}

char *strcpy(char *dest, const char *src)
{
    char *res = dest;
    while (*src != 0)
    {
        *dest = *src;
        src += 1;
        dest += 1;
    }

    *dest = 0x00;

    return res;
}

int strncmp(const char *str1, const char *str2, int n)
{
    unsigned char u1, u2;

    while (n-- > 0)
    {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

char *strncpy(char *dest, const char *src, int count)
{
    int i = 0;
    for (i = 0; i < count - 1; i++)
    {
        if (src[i] == 0x00)
            break;

        dest[i] = src[i];
    }

    dest[i] = 0x00;
    return dest;
}

char *sp = 0;
char *strtok(char *str, const char *deliminators)
{
    int i = 0;
    int len = strlen(deliminators);

    if (!str && !sp)
        return 0;

        
    if (str && !sp)
    {
        sp = str;
    }

    char *p_start = sp;

    while (1)
    {
        for (i = 0; i < len; i++)
        {
            if (*p_start == deliminators[i])
            {
                p_start++;
                break;
            }
        }

        if (i == len)
        {
            sp = p_start;
            break;
        }
    }
    if (*sp == '\0')
    {
        sp = 0;
        return sp;
    }

    // Find end of substring
    while(*sp != '\0')
    {
        for ( i = 0; i < len; i++)
        {
            if(*sp ==deliminators[i])
            {
                *sp= '\0';
                break;
            }
        }
        sp++;
        if(i < len)
            break; 
    }
    return p_start;

}
