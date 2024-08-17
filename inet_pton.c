#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

int
main(int argc, char* argv[])
{
    struct sockaddr_in addr;
    char buf[INET6_ADDRSTRLEN];
    /* inet_pton() 函数将点分位的ipv4或者ipv6地址转换成网络字节序整数表示的地址 */
    /* 该函数是可重入的，因为结果保存在由调用方提供的内存中（dst） */
    int ret = inet_pton(AF_INET, "192.168.1.1", &addr.sin_addr);
    if (ret!= 1) {
        printf("inet_pton failed: %s\n", strerror(errno));
        return 1;
    }

    /* inet_ntop() 函数将网络字节序整数表示的地址转换成点分位的ipv4或者ipv6地址 */
    /* 该函数是可重入的，因为结果保存在由调用方提供的内存中（buf），与之对应的不可重入函数是 inet_ntoa() */
    inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf));
    printf("IP address: %s\n", buf);

    return 0;
}