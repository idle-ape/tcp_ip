#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <bits/sigaction.h>

void read_proc(int sig)
{
    int status;
    pid_t id = waitpid(-1, &status, WNOHANG);
    if (WIFEXITED(status))
    {
        printf("remove child process: %d, child send: %d\n", id, WEXITSTATUS(status));
    }
}

int main(int argc, char *argv[])
{
    pid_t pid;
    struct sigaction act;
    act.sa_handler = read_proc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGCHLD, &act, 0);

    pid = fork();
    if (pid == 0)
    {
        puts("Hi! I'm child process 1");
        sleep(10);
        return 12;
    }
    else
    {
        printf("child process pid: %d\n", pid);
        pid = fork();
        if (pid == 0)
        {
            puts("Hi! I'm child process 2");
            sleep(8);
            exit(24);
        }
        else
        {
            printf("child process pid: %d\n", pid);
            for (size_t i = 0; i < 5; i++)
            {
                puts("waiting...");
                sleep(5);
            }
        }
    }

    return 0;
}