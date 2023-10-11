// I/O流分离带来的EOF问题

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include "error_handling.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    int client_socket;
    struct sockaddr_in addr;
    socklen_t socket_len;
    ssize_t read_len;
    char message[BUF_SIZE];
    FILE *rfp, *wfp;
    if (argc != 3)
    {
        error_handling("Usage: %s <IP> <Port>", argv[0]);
    }

    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        error_handling("socket() err: %s", strerror(errno));
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    socket_len = sizeof(addr);
    if (connect(client_socket, (struct sockaddr *)&addr, socket_len) == -1)
    {
        error_handling("connect() err: %s\n", strerror(errno));
    }

    rfp = fdopen(client_socket, "r");
    wfp = fdopen(client_socket, "w");

    while (1)
    {
        if (fgets(message, BUF_SIZE, rfp) == NULL) // 收到服务端的EOF后，推出循环
        {
            break;
        }
        fputs(message, stdout);
        fflush(stdout);
    }

    fputs("FROM CLIENT: Thank you~", wfp); // 收到EOF后发送消息给服务端
    fflush(wfp);
    fclose(rfp);
    fclose(wfp);
    return 0;
}