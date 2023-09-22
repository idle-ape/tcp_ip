// 获取 socket 的类型
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#include "error_handling.h"

int main(int argc, char *argv[])
{
    int tcp_socket, udp_socket, socket_type;
    socklen_t optlen;
    int state;
    optlen = sizeof(socket_type);

    tcp_socket = socket(PF_INET, SOCK_STREAM, 0);
    udp_socket = socket(PF_INET, SOCK_DGRAM, 0);
    printf("SOCK_STREAM: %d, SOCK_DGRAM: %d\n", SOCK_STREAM, SOCK_DGRAM);

    if (getsockopt(tcp_socket, SOL_SOCKET, SO_TYPE, (void *)&socket_type, &optlen) == -1)
    {
        error_handling("getsockopt() error: %s", strerror(errno));
    }
    printf("tcp socket type: %d\n", socket_type);

    if (getsockopt(udp_socket, SOL_SOCKET, SO_TYPE, (void *)&socket_type, &optlen) == -1)
    {
        error_handling("getsockopt() error: %s", strerror(errno));
    }
    printf("udp socket type: %d\n", socket_type);
    return 0;
}