// 基于 tcp 半关闭的文件传输服务端

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#include "error_handling.h"

#define BUF_SIZE 100

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, clnt_addr;
    // 监听 socket、连接 socket
    int serv_sock, clnt_sock;
    socklen_t socklen;
    char content[BUF_SIZE];
    int read_cnt;
    FILE *fp;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        error_handling("socket() error: %s", strerror(errno));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error: %s", strerror(errno));
    }

    if (listen(serv_sock, 512) == -1)
    {
        error_handling("listen() error: %s", strerror(errno));
    }

    fp = fopen("./bin/udp_echo_client", "rb");

    socklen = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &socklen);
    if (clnt_sock == -1)
    {
        error_handling("accept() error: %s", strerror(errno));
    }

    while (1)
    {
        read_cnt = fread(content, 1, BUF_SIZE, fp);
        if (read_cnt < BUF_SIZE)
        {
            write(clnt_sock, content, read_cnt);
            break;
        }
        write(clnt_sock, content, BUF_SIZE);
    }

    // 关闭输出流，向客户端发送EOF，通知到文件传输完成
    if (shutdown(clnt_sock, SHUT_WR) == -1)
    {
        error_handling("shutdown() error: %s", strerror(errno));
    }
    read(clnt_sock, content, BUF_SIZE);
    printf("receive client message: %s\n", content);

    fclose(fp);
    close(clnt_sock);
    close(serv_sock);

    return 0;
}