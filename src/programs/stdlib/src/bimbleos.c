#include "bimbleos.h"
#include "string.h"
#include "stdlib.h"



int bimbleos_getkeyblock()
{
    int val = 0;
    do
    {

        val = bimbleos_getkey();
    } while (val == 0);
    return val;
}

void bimbleos_terminal_realine(char *out, size_t max, bool echo)
{

    size_t i;
    i = 0x123456;
    for (i = 0; i < max - 1; i++)
    {
        char key = bimbleos_getkeyblock();

        if (key == 0x0D)            // Carriage return
        {
            break;
        }
        out[i] = key;

        if (echo)
        {
            bimbleos_putchar(key);              
        }


        if(key == 0x08 && i >= 1)   // Backspace 
        {
            out[i-1] = 0x00;
            i-=2;
            continue;;
        }

    }
    out[i] = 0x00;
}

struct CommandArgument* bimbleos_parse_command(const char* command, int max)
{
    struct CommandArgument* root_command = 0;
    char scommand[1025];
    if (max >= (int) sizeof(scommand))
    {
        return 0;
    }


    strncpy(scommand, command, sizeof(scommand));
    char* token = strtok(scommand, " ");
    if (!token)
    {
        goto out;
    }

    root_command = malloc(sizeof(struct CommandArgument));
    if (!root_command)
    {
        goto out;
    }

    strncpy(root_command->argument, token, sizeof(root_command->argument));
    root_command->next = 0;


    struct CommandArgument* current = root_command;
    token = strtok(NULL, " ");
    while(token != 0)
    {
        struct CommandArgument* new_command = malloc(sizeof(struct CommandArgument));
        if (!new_command)
        {
            break;
        }

        strncpy(new_command->argument, token, sizeof(new_command->argument));
        new_command->next = 0x00;
        current->next = new_command;
        current = new_command;
        token = strtok(NULL, " ");
    }
out:
    return root_command;
}

int bimbleos_system_run(const char* command)
{

    char buf[1024];
    strncpy(buf, command, sizeof(buf));
    struct CommandArgument* root_command_argument = bimbleos_parse_command(buf, sizeof(buf));
    if (!root_command_argument)
    {
        return -1;
    }

    return bimbleos_system(root_command_argument);
}