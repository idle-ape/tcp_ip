#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/sendfile.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: %s <server_ip> <server_port> <file_path>\n", argv[0]);
        return 1;
    }

    int sockfd, clienfd, n;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, 5) < 0)
    {
        perror("listen");
        exit(1);
    }

    clienfd = accept(sockfd, (struct sockaddr *)&client_addr, &len);
    if (clienfd < 0)
    {
        perror("accept");
        exit(1);
    }

    FILE *fp = fopen(argv[3], "rb");
    if (fp == NULL)
    {
        perror("fopen");
        exit(1);
    }

    /*
    零拷贝方式发送文件，使用 sendfile 函数
    函数签名：ssize_t sendfile(int __out_fd, int __in_fd, off_t *__offset, size_t __count)
    in_fd：要发送的文件描述符，它必须是一个支持类似 mmap 函数的文件描述符，必须指向真是的文件，不能是管道或 socket。
    out_fd：发送文件的描述符，它必须是一个 socket。
    offset：指定从文件流的哪个位置开始读，为空表示默认的起始位置
    count：指定要发送的字节数
    */
    sendfile(clienfd, fileno(fp), NULL, 1024);

    fclose(fp);
    close(clienfd);
    close(sockfd);

    return 0;
}