#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


int main(int argc, char *argv[])
{
    int pid = fork();
    if(pid==0)
    {
        int i=0,j=1;
        for(i=0;i<1000000;i++)
        {
            j*=2;
        }
    }
    else{
        sleep(1000);
    }
    wait(0);
    if(pid!=0)
        //getprocesstimedetails();
    exit(0);
}