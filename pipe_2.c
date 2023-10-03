// 通过管道实现进程间的双向通信
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int fds[2];
    char str[] = "Who are you?";
    char str2[] = "Thank you for your message";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds);
    pid = fork();
    if (pid == 0)
    {
        write(fds[1], str, sizeof(str));
        sleep(2);
        read(fds[0], buf, BUF_SIZE);
        printf("child proc output: %s\n", buf);
    }
    else
    {
        read(fds[0], buf, BUF_SIZE);
        printf("parent proc output: %s\n", buf);
        write(fds[1], str2, sizeof(str2));
        sleep(3);
    }
    return 0;
}