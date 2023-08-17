#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
int main(void)
{

    printf(1, "Total Number of Open Files: %d\n", numOpenFiles());
    int fd;
    fd = open("backup", O_CREATE | O_RDWR);
    printf(1, "Total Number of Open Files: %d\n", numOpenFiles());
    close(fd);
    printf(1, "Total Number of Open Files: %d\n", numOpenFiles());
    exit(0);
}
