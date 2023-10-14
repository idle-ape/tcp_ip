// 多线程聊天室服务端
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "error_handling.h"

#define BUF_SIZE 100
#define MAX_CLNT 255
#define MAX_EVENTS 255

int clnt_fds[MAX_CLNT] = {0}; // 客户端集合
pthread_t tid;
pthread_mutex_t mutex;
int fd_cnt = 0; // 已连接客户端总数

void *thread_send_message(void *arg);

int main(int argc, char *argv[])
{
    int lisfd, clntfd, epfd;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t addr_sz;
    struct epoll_event event, events[MAX_EVENTS];

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    lisfd = socket(PF_INET, SOCK_STREAM, 0);
    if (lisfd == -1)
    {
        error_handling("socket() error: %s", strerror(errno));
    }
    int reuse = 1;
    setsockopt(lisfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(lisfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error: %s", strerror(errno));
    }

    if (listen(lisfd, 512) == -1)
    {
        error_handling("listen() error: %s", strerror(errno));
    }

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        error_handling("pthread_mutex_init() error: %s", strerror(errno));
    }

    epfd = epoll_create(512);
    event.data.fd = lisfd;
    event.events = EPOLLIN;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lisfd, &event) == -1)
    {
        error_handling("epoll_ctl() error: %s", strerror(errno));
    }

    while (1)
    {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n == -1)
        {
            error_handling("epoll_wait() error: %s", strerror(errno));
        }

        for (size_t i = 0; i < n; i++)
        {
            if (events[i].data.fd == lisfd)
            {
                addr_sz = sizeof(clnt_addr);
                clntfd = accept(lisfd, (struct sockaddr *)&clnt_addr, &addr_sz);
                if (clntfd == -1)
                {
                    close(clntfd);
                    continue;
                }
                printf("new connection: %d\n", clntfd);

                pthread_mutex_lock(&mutex);
                clnt_fds[fd_cnt] = clntfd;
                fd_cnt += 1;
                pthread_mutex_unlock(&mutex);

                // 每个连接一个线程单独处理
                if (pthread_create(&tid, NULL, thread_send_message, (void *)&clntfd) == -1)
                {
                    close(clntfd);
                    continue;
                }
                pthread_detach(tid); // 不能用 ptherad_join()，因为该函数会阻塞主进程
            }
        }
    }
    close(lisfd);
    return 0;
}

void *thread_send_message(void *arg)
{
    int clntfd = *((int *)arg);
    printf("new thread to handle %d\n", clntfd);
    char message[BUF_SIZE];

    while (1)
    {
        int read_len = read(clntfd, message, BUF_SIZE);
        if (read_len == 0)
        {
            printf("client %d gone!", clntfd);
            pthread_mutex_lock(&mutex);
            // 将断开连接的客户端从数组中移除
            for (size_t i = 0; i < fd_cnt; i++)
            {
                if (clnt_fds[i] == clntfd)
                {
                    while (i < fd_cnt - 1)
                    {
                        clnt_fds[i] = clnt_fds[i + 1];
                        i += 1;
                    }
                    break;
                }
            }
            fd_cnt--;
            pthread_mutex_unlock(&mutex);

            close(clntfd);
            return NULL;
        }

        message[read_len] = 0;
        printf("receive message %s from %d\n", message, clntfd);
        // 给其他客户端发消息
        for (size_t i = 0; i < fd_cnt; i++)
        {
            pthread_mutex_lock(&mutex);
            if (clnt_fds[i] != clntfd)
            {
                printf("send message %s to %d\n", message, clnt_fds[i]);
                write(clnt_fds[i], message, strlen(message));
            }
            pthread_mutex_unlock(&mutex);
        }
    }
}