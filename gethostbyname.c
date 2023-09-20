// 通过域名获取IP地址

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "error_handling.h"

int main(int arvc, char *argv[])
{
    int i;
    struct hostent *host;

    if (arvc != 2)
    {
        printf("Usage: %s <domain>\n", argv[0]);
        exit(1);
    }

    host = gethostbyname(argv[1]);
    if (!host)
    {
        error_handling("gethostbyname() error: %s", strerror(errno));
    }

    printf("Official name: %s\n", host->h_name);
    for (i = 0; host->h_aliases[i]; i++)
    {
        printf("testtest");
        printf("Alias %d: %s\n", i + 1, host->h_aliases[i]);
    }
    printf("Address type %d\n", host->h_addrtype);
    for (i = 0; host->h_addr_list[i]; i++)
    {
        printf("IP addr %d: %s\n", i + 1, inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));
    }
    return 0;
}