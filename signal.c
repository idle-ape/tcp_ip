#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig)
{
    if (sig == SIGALRM)
    {
        
        puts("timeout!");
    }
    alarm(2);
}

void keycontrol(int sig)
{
    if (sig == SIGINT)
    {
        puts("ctrl+c pressed");
    }
}
int main(int argc, char *argv[])
{
    int i;
    signal(SIGALRM, timeout);
    signal(SIGINT, keycontrol);
    alarm(2); // 2sec后将产生SIGALRM信号。若向该函数传递0，之前对SIGALRM信号的预约将取消。通过该函数预约信号后未指定该信号对应的处理函数，则（通过调用signal函数）终止进程，不做任何处理。

    // 为了查看信号产生和信号处理器的执行，提供每次100秒、共3次的等带时间
    for (size_t i = 0; i < 3; i++)
    {
        puts("wait...");
        sleep(100);
    }
    return 0;
}

/*
root@ubuntu1:~/clang# ./bin/signal
wait...
timeout!
wait...
timeout!
wait...
timeout!
*/

/*
上述代码，按预期是要过300秒、约5分钟后终止程序，但实际执行时只需不要10秒。因为产生信号时，为了调用信号处理器，
将唤醒由于调用sleep函数而进入阻塞状态的进程。而且，进程一旦被唤醒，就不会再进入睡眠状态。即使还未到sleep函数中归档的时间也是如此。
所以上述代码运行不到10秒就会结束，因为每隔两秒就会产生一个SIGALRM信号，每次sleep实际不会到100秒。
*/