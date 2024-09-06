#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>

#define BUFFER_SIZE 10

int setnonblocking(int fd, bool reuse_addr)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl(F_SETFL)");
        return -1;
    }

    if (reuse_addr)
    {
        int optval = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            perror("setsockopt(SO_REUSEADDR)");
            return -1;
        }
    }
    

    return flags;
}

void lt_handler(int epoll_fd, int sockfd, int nfds, struct epoll_event *events)
{
    for(int i = 0; i < nfds; i++)
    {
        /* new client in */
        if (events[i].data.fd == sockfd)
        {
            int client_fd = accept(sockfd, NULL, NULL);
            printf("new client in, fd = %d\n", client_fd);
            if (client_fd < 0)
            {
                perror("accept");
                exit(1);
            }
            struct epoll_event event;
            event.events = EPOLLIN;
            event.data.fd = client_fd;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0)
            {
                perror("epoll_ctl");
                exit(1);
            }
        } else {
            /* client data in*/
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);
            printf("trigger lt, client data in, fd = %d, buffer ptr = %p\n", events[i].data.fd, buffer);
            /* LT 模式下，事件会一直触发，这次没有读完，下次还可以再读 */
            int len = read(events[i].data.fd, buffer, BUFFER_SIZE);
            if (len < 0) {
                perror("read");
                exit(1);
            }
            if (len == 0) {
                /* client closed */
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0) {
                    perror("epoll_ctl");
                    exit(1);
                }
                close(events[i].data.fd);
            } else {
                /* client data out */
                printf("fd: %d, recv: %s\n", events[i].data.fd, buffer);
                len = write(events[i].data.fd, buffer, len);
                if (len < 0) {
                    perror("write");
                    exit(1);
                }
            }
        }
    }
}

void et_handler(int epoll_fd, int sockfd, int nfds, struct epoll_event *events)
{
    for (int i = 0; i < nfds; i++)
    {
        /* new client in */
        if (events[i].data.fd == sockfd)
        {
            int client_fd = accept(sockfd, NULL, NULL);
            printf("new client in, fd = %d\n", client_fd);
            if (client_fd < 0)
            {
                perror("accept");
                exit(1);
            }
            /* ET 模式下，客户端必须设置为非阻塞，否则读或写操作将会因为没有后续的事件而一直处于阻塞状态 */
            if (setnonblocking(client_fd, false) < 0)
            {
                perror("setnonblocking");
                exit(1);
            }
            struct epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = client_fd;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0)
            {
                perror("epoll_ctl");
                exit(1);
            }
        }
        else
        {
            /* client data in*/
            printf("trigger et, client data in, fd = %d\n", events[i].data.fd);
            /* ET 模式下，事件只触发一次，所以这里要将数据全部读出，直到没有数据可读 */
            while (1) {    
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, BUFFER_SIZE);
                int len = read(events[i].data.fd, buffer, BUFFER_SIZE);
                if (len < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        printf("read all data, read later, fd = %d\n", events[i].data.fd);
                        break;
                    } else {
                        perror("read");
                        exit(1);
                    }
                }
                if (len == 0)
                {
                    /* client closed */
                    printf("client closed, fd = %d\n", events[i].data.fd);
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0)
                    {
                        perror("epoll_ctl");
                        exit(1);
                    }
                    close(events[i].data.fd);
                    break;
                }
                else
                {
                    /* client data out */
                    printf("fd: %d, recv: %s\n", events[i].data.fd, buffer);
                    len = write(events[i].data.fd, buffer, len);
                    if (len < 0)
                    {
                        perror("write");
                        exit(1);
                    }
                }
            }
        }
    }
}
    int main(int argc, char *argv[])
{
    int sockfd, epoll_fd, client_fd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    if(setnonblocking(sockfd, true) < 0){
        perror("setnonblocking");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    puts("server listen on :8080...");

    epoll_fd = epoll_create(10);
    if (epoll_fd < 0) {
        perror("epoll_create");
        exit(1);
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = sockfd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event) < 0) {
        perror("epoll_ctl");
        exit(1);
    }

    while(1) {
        struct epoll_event events[10];
        int nfds = epoll_wait(epoll_fd, events, 10, -1);
        if (nfds < 0) {
            perror("epoll_wait");
            exit(1);
        }
        // lt_handler(epoll_fd, sockfd, nfds, events);
        et_handler(epoll_fd, sockfd, nfds, events);
    }
    return 0;
}