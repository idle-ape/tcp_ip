#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_socket;
    int clnt_socket;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    char message[] = "Hello World!\n";

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit;
    }

    // 创建监听 socket
    serv_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_socket == -1)
    {
        error_handling("socket() error");
    }

    int reuse = 1;
    if (setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        error_handling("setsocketopt() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // bind
    if (bind(serv_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error");
    }

    // listen
    if (listen(serv_socket, 5) == -1)
    {
        error_handling("listen() error");
    }

    clnt_addr_size = sizeof(clnt_addr);
    // clnt_socket 连接socket
    clnt_socket = accept(serv_socket, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_socket == -1)
    {
        error_handling("accpet() error");
    }

    write(clnt_socket, message, sizeof(message));
    close(clnt_socket);
    close(serv_socket);

    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(-1);
}