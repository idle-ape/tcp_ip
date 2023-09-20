#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>

int main()
{
    int clnt_socket = socket(PF_INET, SOCK_STREAM, 0);
    // char *svr_addr = "127.0.0.1";
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clnt_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("connetct() error: %s\n", strerror(errno));
        exit(1);
    }

    char buf[20];
    if (read(clnt_socket, &buf, sizeof(buf)) == -1)
    {
        printf("read() error: %s\n", strerror(errno));
        exit(1);
    }

    printf("read: %s\n", buf);
}