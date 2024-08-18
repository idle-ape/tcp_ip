// socket 相关api及选项
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

    int
    main(int argc, char *argv[])
{
    int socketfd;
    socklen_t socket_len;
    char buffer[1024];
    struct sockaddr_in server_addr, client_addr;

    /* 未命名socket */
    socketfd = socket(PF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
        perror("create socket error");

    int reuse_addr = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)); // 允许端口复用

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); // 网络序的端口号
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* 将socket和socket地址绑定，即给socket命名 */
    if(bind(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        perror("bind socket error");

    /* 
    常见的errno为
        EACCESS：绑定的地址是受保护的地址，仅超级用户能够访问，比如普通用户将socket地址绑定到知名服务的端口上（0-1023）
        EADDRINUSE：指定的地址已经被使用，比如将socket绑定到一个处于 TIME_WAIT 状态的端口上
    */
    /* 
    listen函数的第二个参数backlog
        内核版本2.2之前，表示所有处于半连接状态（SYN_RCVD）和全连接状态（ESTABLISHED）的socket的上限
        内核版本2.2之后，表示处于全连接状态的socket的上限，处于半连接状态的socket的上限则由 /proc/sys/net/ipv4/tcp_max_syn_backlog 决定
    */
    if (listen(socketfd, 5) == -1)
        perror("listen socket error"); 

    while (1) {
        /*
        如果这里不调用 accept 从全连接队列中取出建立好的连接socket，则最多只能有 backlog + 1 个客户端可以连接
        root@ubuntu1:~/tcpip# ss -lnt | grep 8080
        LISTEN 6      5          127.0.0.1:8080       0.0.0.0:*
        */

        // socket_len = sizeof(client_addr);
        // int clientfd = accept(socketfd, (struct sockaddr *)&client_addr, &socket_len);
        // if(clientfd == -1)
        //     perror("accept socket error");
        // printf("accept a client, ip=%s, port=%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(socketfd);
    return 0;
}