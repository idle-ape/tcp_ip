// 基于select的I/O复用回声服务端
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include "error_handling.h"

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock, fd_max;
    struct sockaddr_in serv_addr, clnt_addr;
    fd_set reads, cpy_reads;
    struct timeval timeout;
    char buf[BUF_SIZE];

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        error_handling("socket() error: %s", strerror(errno));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error: %s", strerror(errno));
    }

    if (listen(serv_sock, 100) == -1)
    {
        error_handling("listen() error: %s", strerror(errno));
    }

    fd_max = serv_sock;
    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);

    while (1)
    {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        cpy_reads = reads;
        int result = select(1000, &cpy_reads, 0, 0, &timeout);
        if (result == -1)
        {
            printf("select() error: %s\n", strerror(errno));
            continue;
        }
        if (result == 0)
        {
            // printf("timeout!!!\n");
            continue;
        }

        for (size_t i = 0; i < fd_max + 1; i++)
        {
            if (FD_ISSET(i, &cpy_reads))
            {
                if (i == serv_sock)
                {
                    // 新连接进来了
                    socklen_t addr_sz = sizeof(clnt_addr);
                    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &addr_sz);
                    if (clnt_sock == -1)
                    {
                        printf("accpet() error: %s\n", strerror(errno));
                        close(clnt_sock);
                        continue;
                    }
                    // 保存当前最大的fd
                    if (clnt_sock > fd_max)
                        fd_max = clnt_sock;

                    printf("new connection come in, fd: %d\n", clnt_sock);
                    FD_SET(clnt_sock, &reads);
                }
                else
                {
                    // 连接上有数据可读
                    int read_len = read(i, buf, BUF_SIZE);
                    if (read_len == 0)
                    {
                        FD_CLR(i, &reads);
                        printf("fd %ld closed\n", i);
                        close(i);
                        continue;
                    }

                    printf("Message from fd %ld: %s\n", i, buf);
                    write(i, buf, read_len);
                    buf[read_len] = 0;
                }
            }
        }
    }
}