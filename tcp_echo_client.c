// 基于tcp的回声客户端

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *format, char *message);

int main(int argc, char *argv[])
{
    int client_socket;
    struct sockaddr_in addr;
    socklen_t socket_len;
    ssize_t read_len;
    char message[BUF_SIZE];
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

    while (1)
    {
        fputs("Enter Q or q to exit: ", stdout);
        fgets(message, BUF_SIZE, stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        {
            break;
        }

        write(client_socket, &message, sizeof(message));
        read_len = read(client_socket, &message, sizeof(message));
        printf("Message from server: %s", message);
        message[read_len] = 0;
    }
    close(client_socket);
}

void error_handling(char *format, char *message)
{
    fprintf(stderr, format, message);
    fputc('\n', stderr);
    exit(1);
}