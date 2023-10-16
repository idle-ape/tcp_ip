// 多线程聊天室客户端
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

pthread_t tid1, tid2;
pthread_mutex_t mutex;

void *thread_read_message(void *arg); // 接收聊天室的消息
void *thread_send_message(void *arg); // 向聊天室发送消息

int sock;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    struct sockaddr_in serv_addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("connect() error: %s", strerror(errno));
    }

    pthread_mutex_init(&mutex, NULL);

    FILE *rfp, *wfp;
    rfp = fdopen(sock, "r");
    wfp = fdopen(sock, "w");

    pthread_create(&tid1, NULL, thread_read_message, (void *)rfp);
    pthread_create(&tid2, NULL, thread_send_message, (void *)wfp);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    return 0;
}

void *thread_read_message(void *arg)
{
    FILE *rfp = (FILE *)arg;
    char buf[BUF_SIZE];
    while (fgets(buf, BUF_SIZE, rfp) != NULL)
    {
        printf("new message: %s", buf);
    }
    fclose(rfp);
}

void *thread_send_message(void *arg)
{
    FILE *wfp = (FILE *)arg;
    int fd = fileno(wfp);
    char buf[BUF_SIZE];
    while (1)
    {
        fgets(buf, BUF_SIZE, stdin);
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
        {
            break;
        }

        fputs(buf, wfp);
        fflush(wfp);
    }
    shutdown(fd, SHUT_WR);
    fclose(wfp);
    return NULL;
}