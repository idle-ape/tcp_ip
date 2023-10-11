// 使用 dup/dup2 复制文件描述符
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int cfd1, cfd2;
    char str1[] = "Hi~\n";
    char str2[] = "It's a nice day~\n";

    cfd1 = dup(1);
    cfd2 = dup2(1, 7);
    printf("fd1=%d, fd2=%d\n", cfd1, cfd2);

    write(cfd1, str1, strlen(str1));
    write(cfd2, str2, strlen(str2));

    close(cfd1);
    close(cfd2);

    write(1, str1, strlen(str1));
    write(1, str2, strlen(str2));
    return 0;
}