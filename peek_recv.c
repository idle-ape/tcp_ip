// 通过MSG_PEEK|MSG_DONTWAIT标志检查输入缓存区中是否有数据可读
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "error_handling.h"

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t addr_sz;
    int listen_sock, conn_sock;
    char buf[BUF_SIZE];
    int read_len;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1)
    {
        error_handling("socket() error: %s", strerror(errno));
    }

    if (bind(listen_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error: %s", strerror(errno));
    }

    if (listen(listen_sock, 100) == -1)
    {
        error_handling("listen() error: %s", strerror(errno));
    }

    addr_sz = sizeof(clnt_addr);
    conn_sock = accept(listen_sock, (struct sockaddr *)&clnt_addr, &addr_sz);
    if (conn_sock == -1)
    {
        error_handling("accept() error: %s", strerror(errno));
    }

    while (1)
    {
        read_len = recv(conn_sock, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT);
        if (read_len > 0)
        {
            break;
        }
    }
    buf[read_len] = 0;
    printf("buffering %d bytes: %s", read_len, buf);

    read_len = recv(conn_sock, buf, BUF_SIZE, 0);
    buf[read_len] = 0;
    printf("read again: %s, len: %d\n", buf, read_len);
    close(conn_sock);
    close(listen_sock);
    return 0;
}