#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include<libgen.h>

#define MAX_EVENTS_NUMBER 1024
#define BUFFER_SIZE 1024

struct fds 
{
    int epollfd;
    int sockfd;
    pthread_t tid;
};

int setnonblocking(int fd) 
{
    int old_flags = fcntl(fd, F_GETFL);
    int new_flags = old_flags | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flags);
    return old_flags;
}

/* 将 fd 上的 EPOLLIN 和 EPOLLET 事件注册到 epollfd 指示的 epoll 内核事件表中，参数 oneshot 指定是否注册 fd 上的 EPOLLONESHOT 事件。 */
void addfd(int epollfd, int fd, bool oneshot) 
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if (oneshot) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void reset_oneshot(int epollfd, int fd) 
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void *woker(void *arg)
{
    int sockfd = ((struct fds *)arg)->sockfd;
    int epollfd = ((struct fds *)arg)->epollfd;
    pthread_t tid = ((struct fds *)arg)->tid;

    printf("thread %ld to receive data from fd: %d\n", tid, sockfd);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    while (1)
    {
        int nread = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (nread == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                printf("no data to read, read later\n");
                /* 重置 sockfd 上的 EPOLLONESHOT 事件，让 epoll 有机会再次检测到该 socket 上的事件，进而使得其他线程有机会为该 socket 服务。 */
                /* 如果处理完不重置 EPOLLONESHOT 事件，则该 socket 上的事件将不再会被触发，因为只会触发一次 */
                reset_oneshot(epollfd, sockfd);
                break;
            }
            else
            {
                printf("read error: %s\n", strerror(errno));
                break;
            }
        } else if (nread == 0) {
            printf("connection closed by peer\n");
            close(sockfd);
            break;
        } else {
            printf("received data: %s\n", buffer);
            sleep(5);
        }
    }
    pthread_exit(NULL);
    printf("exit thread to receive data from fd: %d\n", sockfd);
}

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        printf("usage: %s ip port\n", basename(argv[0]));
        return 1;
    }
    
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("create socket error: %s\n", strerror(errno));
        return 1;
    }
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(ip, &server_addr.sin_addr);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        printf("bind socket error: %s\n", strerror(errno));
        return 1;
    }

    if (listen(sockfd, 5) == -1)
    {
        printf("listen socket error: %s\n", strerror(errno));
        return 1;
    }

    int epollfd = epoll_create(MAX_EVENTS_NUMBER);
    if (epollfd == -1)
    {
        printf("create epoll error: %s\n", strerror(errno));
        return 1;
    }

    addfd(epollfd, sockfd, false);
    struct epoll_event events[MAX_EVENTS_NUMBER];
    while(1)
    {
        int ret = epoll_wait(epollfd, events, MAX_EVENTS_NUMBER, -1);
        if (ret == -1)
        {
            printf("epoll wait error: %s\n", strerror(errno));
            return 1;
        }
        for (int i = 0; i < ret; i++)
        {
            int fd = events[i].data.fd;
            if (fd == sockfd)
            {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (connfd == -1)
                {
                    printf("accept socket error: %s\n", strerror(errno));
                    continue;
                }
                printf("new connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                addfd(epollfd, connfd, true);
            } else if (events[i].events & EPOLLIN) {
                pthread_t tid;
                struct fds fds = {epollfd, fd, tid};
                /* 新启动一个新的工作线程为 socket 服务 */
                pthread_create(&tid, NULL, woker, &fds);
            } else {
                printf("something else happened\n");
            }
        }
    }
    close(sockfd);
    close(epollfd);
    return 0;
}