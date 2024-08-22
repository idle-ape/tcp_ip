#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int fd1, fd2;
    fd1 = open(argv[1], O_RDONLY);
    fd2 = creat(argv[2], 0644);

    /*
        splice的版本要求：Linux 2.6.17, glibc 2.5
    */
    splice(fd1, NULL, fd2, NULL, 1024, 0);

    close(fd1);
    close(fd2);

    return 0;
}