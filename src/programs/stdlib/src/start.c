#include "bimbleos.h"


extern int main(int argc, char** argv);
void c_start(){

    struct ProcessArguments arguments;
    bimbleos_process_get_arguments(&arguments);

    int res  = main(arguments.argc,arguments.argv);
    
    if(res == 0)
    {

    }

}