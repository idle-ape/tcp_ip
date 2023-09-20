#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    char *addr = "127.232.124.79";
    struct sockaddr_in addr_inet;

    // inet_aton 将点分表示的IP地址转成网络序，并更新到 sockaddr_in 的 sin_addr 字段
    if (!inet_aton(addr, &addr_inet.sin_addr))
    {
        fputs("conversion err", stderr);
        fputc('\n', stderr);
        exit(1);
    }
    else
    {
        printf("Netword ordered integer addr: %#x\n", addr_inet.sin_addr.s_addr);
    }
    return 0;
}