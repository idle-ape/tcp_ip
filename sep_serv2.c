// 复制文件描符后的I/O流分离，解决无法半关闭的问题

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

    client_socket_len = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_socket_len);
    if (client_socket == -1)
    {
        error_handling("accept() err: %s\n", strerror(errno));
    }
    printf("new connetion come in, fd: %d\n", client_socket);
    rfp = fdopen(client_socket, "r");
    wfp = fdopen(dup(client_socket), "w");

    fputs("FROM SERVER: Hi, client~\n", wfp);
    fputs("I love all of the world~\n", wfp);
    fputs("You are awesome~\n", wfp);
    fflush(wfp);

    // 通过调用 shutdown 方法实现半关闭
    // 调用 shutdown 方法时，无论套接字复制出多少个文件描数据，都将进入半关闭状态，同时传递EOF
    shutdown(fileno(wfp), SHUT_WR);
    fclose(wfp);

    fgets(message, BUF_SIZE, rfp);
    fputs(message, stdout);
    fputc('\n', stdout);
    fclose(rfp);

    close(server_socket);
    return 0;
}