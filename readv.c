// 通过 readv() 函数，将读取到的数据分别由多个缓冲区接收
#include <stdlib.h>
#include <stdio.h>
#include <sys/uio.h>

#define BUF_SIZE 100

int main(int argc, char *argv[])
{
    struct iovec vec[2];
    /* 初始化数组，等价于 char buf1[BUF_SIZE] = {0,0,0,0,0,...}  BUF_SIZE个0
    https://stackoverflow.com/questions/30032400/initialize-char-array-in-c
    */
    char buf1[BUF_SIZE] = {0}; 
    char buf2[BUF_SIZE] = {0};

    vec[0].iov_base = buf1;
    vec[0].iov_len = 5;
    vec[1].iov_base = buf2;
    vec[1].iov_len = BUF_SIZE;

    int read_len = readv(0, vec, 2);
    printf("read len: %d\n", read_len);
    printf("first message: %s\n", buf1);
    printf("second message: %s\n", buf2);
}