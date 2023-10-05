// 通过recv()函数以及MSG_OOB标志接收带外数据
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include "error_handling.h"
#include <bits/sigaction.h>

#define BUF_SIZE 30

void read_oob(int sig);

int clnt_sock, sock;

int main(int argc, char *argv[])
{
    char buf[BUF_SIZE];
    struct sockaddr_in serv_addr, clnt_addr;
    struct sigaction act;
    int read_len;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    act.sa_handler = read_oob;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        error_handling("socket() error: %s\n", strerror(errno));
    }

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error: %s\n", strerror(errno));
    }

    if (listen(sock, 100) == -1)
    {
        error_handling("listen() error: %s\n", strerror(errno));
    }

    // while (1)
    // {
    //     cpy_reads = reads;
    //     timeout.tv_sec = 1;
    //     timeout.tv_usec = 0;
    //     int result = select(100, &cpy_reads, 0, 0, &timeout);
    //     if (result == -1)
    //     {
    //         printf("select() error: %s\n", strerror(errno));
    //         continue;
    //     }
    //     if (result == 0)
    //     {
    //         continue; // timeout
    //     }
    //     for (size_t i = 0; i < fd_max + 1; i++)
    //     {
    //         if (FD_ISSET(i, &cpy_reads))
    //         {
    //             if (i == sock)
    //             {
    //                 // new connection
    //                 socklen_t addr_sz = sizeof(clnt_addr);
    //                 clnt_sock = accept(sock, (struct sockaddr *)&clnt_addr, &addr_sz);
    //                 if (clnt_sock == -1)
    //                 {
    //                     close(clnt_sock);
    //                     continue;
    //                 }
    //                 if (clnt_sock > fd_max)
    //                 {
    //                     fd_max = clnt_sock;
    //                 }
    //                 printf("new connecttion, fd: %d\n", clnt_sock);
    //                 fcntl(clnt_sock, __F_SETOWN, getpid());
    //                 FD_SET(clnt_sock, &reads);
    //             }
    //             else
    //             {
    //                 // data come in
    //                 read_len = recv(i, buf, BUF_SIZE, 0);
    //                 if (read_len == 0)
    //                 {
    //                     FD_CLR(i, &reads);
    //                     close(i);
    //                     continue;
    //                 }
    //                 buf[read_len] = 0;
    //                 printf("recv data: %s, len: %d\n", buf, read_len);
    //             }
    //         }
    //     }
    // }

    socklen_t addr_sz = sizeof(clnt_addr);
    clnt_sock = accept(sock, (struct sockaddr *)&clnt_addr, &addr_sz);
    if (clnt_sock == -1)
    {
        error_handling("accept() error: %s", strerror(errno));
    }
    fcntl(clnt_sock, __F_SETOWN, getpid());
    sigaction(SIGURG, &act, 0);

    while ((read_len = recv(clnt_sock, buf, sizeof(buf), 0)) != 0)
    {
        if (read_len == -1)
        {
            continue;
        }
        buf[read_len] = 0;
        printf("recv normal data: %s\n", buf);
    }
    close(clnt_sock);
    close(sock);
    sleep(5);
    return 0;
}

void read_oob(int sig)
{
    if (sig == SIGURG)
    {
        char buf[BUF_SIZE];
        int l = recv(clnt_sock, buf, sizeof(buf) - 1, MSG_OOB);
        buf[l] = 0;
        printf("recv oob data: %s, len: %d\n", buf, l);
    }
}