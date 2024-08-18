#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>

int main() {
    int fds[2];
    /* socketpair 创建的这对文件描述符，既能读又能写，不像 pipe 函数那样，第一个fd只能读，第二个fd只能写 */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1)
        perror("socketpair");

    pid_t pid = fork();
    if (pid == -1)
        perror("fork");

    if (pid == 0) {
        // child process
        close(fds[0]);
        char msg[] = "Hello, world!";
        write(fds[1], msg, strlen(msg));
        close(fds[1]);
        exit(0);
    } else {
        // parent process
        close(fds[1]);
        char buf[1024];
        read(fds[0], buf, sizeof(buf));
        printf("Received: %s\n", buf);
        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
    return 0;
}