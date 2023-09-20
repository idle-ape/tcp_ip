// 基于未连接udp套接字的回声客户端

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "error_handling.h"

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int clnt_sock, str_len;
    char message[BUF_SIZE];
    struct sockaddr_in serv_addr, from_addr;
    socklen_t socklen;

    if (argc != 3)
    {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    clnt_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (clnt_sock == -1)
    {
        error_handling("socket() error: %s", strerror(errno));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    while (1)
    {
        printf("input something here(q or Q to quit): ");
        fgets(message, BUF_SIZE, stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        {
            break;
        }

        socklen = sizeof(serv_addr);
        str_len = sendto(clnt_sock, message, strlen(message), 0, (struct sockaddr *)&serv_addr, socklen);
        socklen = sizeof(from_addr);
        recvfrom(clnt_sock, message, BUF_SIZE, 0, (struct sockaddr *)&from_addr, &socklen);
        printf("receive from server: %s", message);
    }

    close(clnt_sock);
    return 0;
}