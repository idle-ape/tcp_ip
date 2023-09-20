#include <stdio.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    char *addr1 = "1.2.3.4";
    char *addr2 = "1.2.3.256";

    // inet_addr 将点分表示的IP地址转成网络序
    unsigned long conv_addr = inet_addr(addr1);
    if (conv_addr == INADDR_NONE)
    {
        printf("Error occured\n");
    }
    else
    {
        printf("Netword ordered integer addr: %#lx\n", conv_addr);
    }

    conv_addr = inet_addr(addr2);
    if (conv_addr == INADDR_NONE)
    {
        printf("Error occured\n");
    }
    else
    {
        printf("Netword ordered integer addr: %#lx\n", conv_addr);
    }
}