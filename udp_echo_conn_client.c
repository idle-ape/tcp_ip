// 基于已连接udp套接字的回声客户端

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "error_handling.h"

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int clnt_sock, str_len;
    char message[BUF_SIZE];
    struct sockaddr_in serv_addr, from_addr;
    socklen_t socklen;

    if (argc != 3)
    {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    clnt_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (clnt_sock == -1)
    {
        error_handling("socket() error: %s", strerror(errno));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    /**
     * udp绑定IP地址和端口号是在调用 sendto 函数时进行的，通过 sendto 函数传输数据可大致分为以下3个阶段：
     * 1、向udp套接字注册目标IP和端口号
     * 2、传输数据
     * 3、删除udp套接字中注册的目标地址信息
     *
     * 每次调用 sendto 函数时都要重复上述过程，因此要与一个主机长时间通信时，将udp套接字编程已连接套接字会提高效率。
     * 上述三个阶段中，第一个和第三个阶段占整个通信过程近1/3的时间，缩短这部分时间将大大提高整体性能。
     */

    // 多了这一步 connect，即可创建已连接udp套接字
    if (connect(clnt_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("connect() error: %s", strerror(errno));
    }

    while (1)
    {
        printf("input something here(q or Q to quit): ");
        fgets(message, BUF_SIZE, stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        {
            break;
        }

        socklen = sizeof(serv_addr);
        // 因为是已连接udp套接字，所以可以像tcp套接字一样，已经有了目标地址信息，每次调用sendto函数时只需要传输数据，还可以使用 write、read 函数进行通信
        // str_len = sendto(clnt_sock, message, strlen(message), 0, (struct sockaddr *)&serv_addr, socklen);
        str_len = write(clnt_sock, message, strlen(message));
        socklen = sizeof(from_addr);
        // recvfrom(clnt_sock, message, BUF_SIZE, 0, (struct sockaddr *)&from_addr, &socklen);
        read(clnt_sock, message, sizfof(message));
        printf("receive from server: %s", message);
    }

    close(clnt_sock);
    return 0;
}