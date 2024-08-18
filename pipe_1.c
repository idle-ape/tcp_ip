// 通过管道实现进程间的通信
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int fds[2]; /* fds[0] for reading, fds[1] for writing */
    char str[] = "Who are you?";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds);
    pid = fork();
    if (pid == 0)
    {
        sleep(10);
        write(fds[1], str, sizeof(str));
    }
    else
    {
        /*
        read from pipe after 10 seconds, becase the child process has slept for 10 seconds,
        read() will block until the child process writes something to the pipe.
        */
        read(fds[0], buf, BUF_SIZE);
        printf("read from other side: %s\n", buf);
    }
    return 0;
}