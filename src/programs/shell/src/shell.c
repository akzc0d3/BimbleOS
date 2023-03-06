#include "bimbleos.h"
#include "shell.h"
#include "stdio.h"
 
 

int main(int argc, char** argv)
{
    printf("BimbleOS v1.0\n");


    do{
        printf(">");


        char buff[1024];

        bimbleos_terminal_realine(buff,sizeof(buff),true);
        print("\n");
        bimbleos_system_run(buff);
         

        printf("\n");
    }while (1);

    return 0;
     
}
 