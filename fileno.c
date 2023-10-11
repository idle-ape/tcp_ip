// 通过 fileno 函数将 FILE 指针转换为文件描述符
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main()
{
    FILE *fp;
    fp = fopen("./bin/tess.txt", "r");
    int fd = fileno(fp);
    printf("fd: %d\n", fd);

    fclose(fp);
    return 0;
}