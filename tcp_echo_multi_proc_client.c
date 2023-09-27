// 回声客户端多进程版本
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
#include "error_handling.h"

#define BUF_SIZE 30

void read_childproc(int sig);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    struct sigaction act;
    int sock;
    pid_t pid;
    char message[BUF_SIZE];

    if (argc != 3)
    {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    sigemptyset(&act.sa_mask);
    act.sa_handler = read_childproc;
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        error_handling("socket() err: %s", strerror(errno));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    socklen_t addr_sz = sizeof(serv_addr);
    if (connect(sock, (struct sockaddr *)&serv_addr, addr_sz) == -1)
    {
        error_handling("connect() err: %s", strerror(errno));
    }

    pid = fork();
    if (pid == 0)
    {
        write_routine(sock, message);
    }
    else
    {
        read_routine(sock, message);
    }

    close(sock);
    return 0;
}

void read_routine(int sock, char *buf)
{
    while (1)
    {
        int read_len = read(sock, buf, BUF_SIZE);
        if (read_len == 0)
        {
            return;
        }

        buf[read_len] = 0;
        printf("Message from server: %s", buf);
    }
}
void write_routine(int sock, char *buf)
{
    while (1)
    {
        // fputs("Enter Q or q to exit: ", stdout);
        fgets(buf, BUF_SIZE, stdin);
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
        {
            shutdown(sock, SHUT_WR); // 发送EOF
            return;
        }
        write(sock, buf, strlen(buf));
    }
}

void read_childproc(int sig)
{
    if (sig == SIGCHLD)
    {
        int status;
        pid_t id = waitpid(-1, &status, WNOHANG);
        if (WIFEXITED(status))
        {
            printf("child proc %d exit, send: %d\n", id, WEXITSTATUS(status));
        }
    }
}