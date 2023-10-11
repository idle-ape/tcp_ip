// 基于标准I/O函数实现的回声服务端

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "error_handling.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        error_handling("Usage: %s <IP> <Port>", argv[0]);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_socket_len;
    ssize_t read_len;
    char message[BUF_SIZE];
    FILE *rfp, *wfp;

    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        error_handling("socket() err: %s\n", strerror(errno));
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_family = AF_INET;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        error_handling("bind() err: %s", strerror(errno));
    }

    if (listen(server_socket, 5) == -1)
    {
        error_handling("listen() err: %s\n", strerror(errno));
    }

    for (size_t i = 0; i < 5; i++)
    {
        client_socket_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_socket_len);
        if (client_socket == -1)
        {
            error_handling("accept() err: %s\n", strerror(errno));
        }
        printf("new connetion come in, fd: %d\n", client_socket);
        rfp = fdopen(client_socket, "r");
        wfp = fdopen(client_socket, "w");
        while (!feof(rfp))
        {
            fgets(message, BUF_SIZE, rfp);
            printf("get message from client: %s", message);
            fputs(message, wfp);
            fflush(wfp);
        }

        printf("client gone, fd: %d\n", client_socket);
        fclose(rfp);
        fclose(wfp);
    }
    close(server_socket);
    return 0;
}