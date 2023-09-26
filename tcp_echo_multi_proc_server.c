// 回声服务端多进程版本
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

#define BUF_SIZE 1024

void read_childproc(int sig);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    struct sigaction act;
    pid_t pid;
    char buf[BUF_SIZE];
    int read_len;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    act.sa_handler = read_childproc;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    // 注册 SIGCHLD 信号处理函数
    int state = sigaction(SIGCHLD, &act, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 创建监听socket
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        error_handling("socket() err: %s", strerror(errno));
    }

    // 给监听socket绑定ip和端口
    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() err: %s", strerror(errno));
    }

    if (listen(serv_sock, 100) == -1)
    {
        error_handling("listen() err: %s", strerror(errno));
    }

    while (1)
    {
        socklen_t addr_sz = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &addr_sz);
        if (clnt_sock == -1)
        {
            continue;
        }
        else
        {
            printf("new client connected, fd: %d\n", clnt_sock);
        }
        pid = fork();
        if (pid == -1)
        {
            close(clnt_sock);
            continue;
        }

        if (pid == 0) // 子进程运行区域
        {
            close(serv_sock);
            while (read_len = read(clnt_sock, buf, BUF_SIZE) > 0)
            {
                printf("read %s from %s:%d\n", buf, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
                write(clnt_sock, buf, read_len);
            }

            close(clnt_sock);
            puts("client disconnected...");
            return 0;
        }
        else
        {
            close(clnt_sock);
        }
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
            printf("child process %d exit, child send: %d\n", id, WEXITSTATUS(status));
        }
    }
}

/*
通过fork()函数复制文件描述符，是只复制了文件描述符，是否同时复制了套接字呢？
套接字并非进程所有--从严格意义上来说，套接字数据操作及系统，只是进程拥有代表相应套接字的文件描述符。
如果套接字也能复制，那么统一读马口将对应多个套接字，所以套接字不能复制！！！

1个套接字中存在多个文件描述符，即多个文件描述符可以关联到一个套接字，只有关联的文件描述符都终止（销毁）了，才能销毁套接字。
所以上述代码中，父进程的运行区域需要关闭连接socket关联的文件描符；同理，子进程的运行区域需要关闭监听socket关联的文件描述符。
*/