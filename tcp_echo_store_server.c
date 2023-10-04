// 回声服务端多进程版本，并通过子进程保存客户端的信息
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include "error_handling.h"

#define BUF_SIZE 1024

void read_childproc(int sig);

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, clnt_addr;
    int serv_socket, clnt_socket;
    pid_t pid;
    char buf[BUF_SIZE];
    int fds[2];

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_socket == -1)
    {
        error_handling("socket() error: %s", strerror(errno));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serv_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error: %s", strerror(errno));
    }

    if (listen(serv_socket, 100) == -1)
    {
        error_handling("listen() error: %s", strerror(errno));
    }

    pipe(fds);
    pid = fork();
    if (pid == 0)
    {
        int read_len;
        char message[BUF_SIZE];
        int fd = open("/tmp/message_in.txt", O_RDWR, 0666);
        // 接收管理里的消息，写入文件
        while (1)
        {
            read_len = read(fds[0], message, BUF_SIZE);
            if (read_len == 0)
            {
                return 0;
            }
            // printf("receive message from pipe: %s, read len: %d\n", message, read_len);
            size_t write_len = write(fd, message, read_len);
            printf("fwrite len: %ld\n", write_len);
        }
    }

    while (1)
    {
        socklen_t addr_sz = sizeof(clnt_addr);
        clnt_socket = accept(serv_socket, (struct sockaddr *)&clnt_addr, &addr_sz);
        if (clnt_socket == -1)
        {
            close(clnt_socket);
            continue;
        }
        pid = fork();
        if (pid == 0)
        {
            close(serv_socket);
            while (1)
            {
                int read_len = read(clnt_socket, buf, BUF_SIZE);
                if (read_len == 0)
                {
                    printf("client %d close the conn\n", clnt_socket);
                    close(clnt_socket);
                    return 0;
                }
                write(fds[1], buf, read_len);
                write(clnt_socket, buf, read_len);
            }
        }
        else
        {
            close(clnt_socket);
        }
    }
    close(serv_socket);
    return 0;
}