// 基于 epoll 的回声服务端

/*
    一些思考：
        1、为什么不要在连接套接字的fd上注册 EPOLLOUT 事件，如果发送数据也想做成事件触发的方式，该怎么弄？
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "error_handling.h"

#define BUF_SIZE 4 // 调整为4，验证 LT 模式下（默认为LT模式），当缓冲区中有数据时会一直触发事件；而 ET （边缘触发）模式下，只会触发一次
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

                // add the client fd to the epoll
                event.events = EPOLLIN;
                // event.events = EPOLLIN | EPOLLET;
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
                    int read_len = read(events[i].data.fd, buf, BUF_SIZE);
                    if (read_len == 0)
                    {
                        printf("client %d gone.\n", events[i].data.fd);
                        close(events[i].data.fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        continue;
                    }

                    buf[read_len] = 0;
                    printf("receive message from client: %s\n", buf);

                    event.data.ptr = buf;
                    event.data.fd = events[i].data.fd;
                    event.events = EPOLLOUT;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &event); // 修改标识符，等待下一个循环时发送数据，异步处理的精髓
                }
                else if (events[i].events & EPOLLOUT)
                {
                    // char buf[] = (char *)events[i].data.ptr;
                    printf("epoll out, fd: %d, message: %s", events[i].data.fd, buf);
                    write(events[i].data.fd, buf, strlen(buf));
                    event.data.fd = events[i].data.fd;
                    event.events = EPOLLIN;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &event); // 修改标识符，等待下一个循环时接收数据
                }
            }
        }
    }
    close(listenfd);
    close(epfd);
    return 0;
}