#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in addr1, addr2;

    addr1.sin_addr.s_addr = htonl(0x1020304);
    addr2.sin_addr.s_addr = htonl(0x1010101);

    /* 
        inet_ntoa()函数将网络序表示的地址转成点分的IP地址，这里要注意的是，inet_ntoa()函数是不可重入的
        因为该函数内部使用了静态变量（一直占据内存空间，无论函数调用与否，与之对应的是自动自动变量，其随着函数的调用和退出而存在和释放），
        所以不能在多线程环境下使用，多次调用结果会被覆盖。
    */
    char *str_ptr = inet_ntoa(addr1.sin_addr);
    // printf("dotted-decimal notation1: %s\n", str_ptr); /* 1.2.3.4 */
    char *str_ptr2 = inet_ntoa(addr2.sin_addr);
    printf("dotted-decimal notation1: %s\n", str_ptr); /* 1.1.1.1 */
    printf("dotted-decimal notation2: %s\n", str_ptr2); /* 1.1.1.1 */

    return 0;
}