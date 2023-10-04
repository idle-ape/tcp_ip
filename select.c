// 通过select函数实现io复用
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include "error_handling.h"

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    fd_set reads, temps;
    int result, str_len;
    char buf[BUF_SIZE];
    struct timeval timeout;

    FD_ZERO(&reads);
    FD_SET(0, &reads);

    while (1)
    {
        temps = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 500;

        result = select(1, &temps, 0, 0, &timeout);
        if (result == -1)
        {
            error_handling("select() error: %s", strerror(errno));
        }
        else if (result == 0)
        {
            printf("timeout!!!\n");
        }
        else
        {
            if (FD_ISSET(0, &temps))
            {
                str_len = read(0, buf, BUF_SIZE);
                printf("message from console: %s", buf);
            }
        }
    }
    return 0;
}