// 基于 epoll 边缘触发（ET）模式的回声服务端

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "error_handling.h"

#define BUF_SIZE 4
#define MAX_EVENTS 100

int main(int argc, char *argv[])
{
    int listenfd, clntfd, epfd;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t addr_sz;
    struct epoll_event event;
    char buf[BUF_SIZE];

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        error_handling("socket() error: %s", strerror(errno));
    }
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    printf("listen fd: %d\n", listenfd);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error: %s", strerror(errno));
    }

    if (listen(listenfd, 100) == -1)
    {
        error_handling("listen() error: %s", strerror(errno));
    }

    epfd = epoll_create(1024);

    event.events = EPOLLIN;
    event.data.fd = listenfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event) != 0)
    {
        error_handling("epoll_ctl() error: %s", strerror(errno));
    }

    struct epoll_event events[MAX_EVENTS];
    while (1)
    {
        int n = epoll_wait(epfd, events, sizeof(events), -1);
        if (n == -1)
        {
            error_handling("epoll_wait() error: %s", strerror(errno));
        }

        printf("epoll_wait return, events len: %d\n", n);
        for (size_t i = 0; i < n; i++)
        {
            printf("event fd: %d, event: %d\n", events[i].data.fd, events[i].events);
            if (events[i].data.fd == listenfd)
            {
                addr_sz = sizeof(clnt_addr);
                clntfd = accept(listenfd, (struct sockaddr *)&clnt_addr, &addr_sz);
                printf("new connection, client fd: %d\n", clntfd);

                // set client fd nonblocking
                /*
                为何需要将连接socket的文件描述符设置为非阻塞？
                因为边缘触发方式下，以阻塞方式工作的 read&write函数又可能引起服务器端的长时间停顿，因此边缘触发方式中一定要采用非阻塞的方式。
                */
                int flag = fcntl(clntfd, F_GETFL, 0);
                fcntl(clntfd, F_SETFL, flag | O_NONBLOCK);

                // add the client fd to the epoll
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = clntfd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, clntfd, &event) != 0)
                {
                    error_handling("epoll_ctl() error: %s", strerror(errno));
                }
            }
            else
            {
                if (events[i].events & EPOLLIN)
                {
                    // 由于是边缘触发，所以当读事件触发时要一次性读完缓冲区里的数据
                    while (1)
                    {
                        int read_len = read(events[i].data.fd, buf, BUF_SIZE);
                        if (read_len == 0) // EOF
                        {
                            printf("client %d gone.\n", events[i].data.fd);
                            close(events[i].data.fd);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                            break;
                        }
                        if (read_len == -1)
                        {
                            // 验证错误码, 当read()返回 -1，错误码是 EAGAIN 时，表示没有数据可读了
                            if (errno == EAGAIN)
                            {
                                break;
                            }
                        }
                        buf[read_len] = 0;
                        printf("receive message from client: %s\n", buf);

                        write(events[i].data.fd, buf, strlen(buf));
                    }
                }
            }
        }
    }
    close(listenfd);
    close(epfd);
    return 0;
}