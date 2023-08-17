#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[])
{
    if(argc==1 || argc>2)
    {
        printf(2,"Invalid pid\n");
        exit(-1);
    }

    for(int i=0;i<strlen(argv[1]);i++)
    {
        if(!(argv[1][i]>='1' && argv[1][i]<='9'))
        {
            printf(2,"Invalid pid\n");
            exit(-1);
        }
    }
    if(argv[1][0]>='1' && argv[1][0]<='9')
    {
        int pid = atoi(argv[1]);
        printf(2,"Number of files opened: %d\n",numOpenFiles(pid));
        printf(2,"Memory allocated: %d\n",memAlloc(pid));
        getprocesstimedetails(pid); 
        exit(0);
    }
    else
    {
        printf(2,"Invalid pid\n");
        exit(-1);
    }
}