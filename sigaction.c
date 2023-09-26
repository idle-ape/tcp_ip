// 利用 sigaction 函数进行信号处理
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <bits/sigaction.h>

void timeout(int sig)
{
    if (sig == SIGALRM)
    {
        puts("timeout");
    }
    alarm(2);
}

int main(int argc, char *argv[])
{
    int i;
    struct sigaction act;
    act.sa_handler = timeout;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGALRM, &act, 0);

    alarm(2);

    for (size_t i = 0; i < 3; i++)
    {
        puts("wait...");
        sleep(100);
    }
    return 0;
}