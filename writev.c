// 通过 writev() 函数将分散保存在多个缓冲中的数据一并发送
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>

int main(int argc, char *argv[])
{
    struct iovec vec[2];
    char buf1[] = "ABCDEFG";
    char buf2[] = "1234567";
    int str_len;

    vec[0].iov_base = buf1;
    vec[0].iov_len = 3;
    vec[1].iov_base = buf2;
    vec[1].iov_len = 4;

    str_len = writev(1, vec, 2);
    printf("\nwrite bytes: %d\n", str_len);
    return 0;
}