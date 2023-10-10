// 将文件描述符通过 fdopen 函数转换为 FILE 指针
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main()
{
    FILE *fp;
    int fd = open("./bin/tess.txt", O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1)
    {
        fputs("file open error", stdout);
        exit(-1);
    }

    fp = fdopen(fd, "w");
    fputs("Network C programming\n", fp);
    fclose(fp);
    return 0;
}