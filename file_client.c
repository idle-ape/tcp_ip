// 基于 tcp 半关闭的文件传输客户端

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
    struct sockaddr_in serv_addr;
    // 连接 socket
    int clnt_sock;
    socklen_t socklen;
    char content[BUF_SIZE];
    int read_cnt;
    FILE *fp;

    if (argc != 3)
    {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (clnt_sock == -1)
    {
        error_handling("socket() error: %s", strerror(errno));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(clnt_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error: %s", strerror(errno));
    }

    fp = fopen("./bin/udp_echo_client_receive", "wab");

    while ((read_cnt = read(clnt_sock, content, BUF_SIZE)) > 0)
    {
        fwrite(content, 1, read_cnt, fp);
    }

    char message[] = "Thank you!";
    write(clnt_sock, message, sizeof(message));

    fclose(fp);
    close(clnt_sock);

    return 0;
}