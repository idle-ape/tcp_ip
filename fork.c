// fork子进程
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "error_handling.h"

int main(int argc, char *argv[])
{
    __pid_t pid;
    int gval = 10;
    int lval = 20;
    gval++, lval++;

    printf("before fork: [%d, %d]\n", gval, lval);

    pid = fork(); // 父进程调用后返回子进程的ID；子进程调用后返回0
    if (pid == 0)
    {
        gval += 2, lval += 2;
        printf("child proc: [%d, %d]\n", gval, lval);
    }
    else
    {
        gval -= 2, lval -= 2;
        printf("child pid: %d\n", pid);
        printf("parent proc: [%d, %d]\n", gval, lval);
        sleep(30); // sleep 30秒查看子进程是否编程僵尸进程
    }

    return 0;
}

/*
  root@ubuntu1:~# ps -aux  | grep 252906
  root      252906  0.0  0.0      0     0 pts/1    Z+   14:02   0:00 [fork] <defunct>
  root      252908  0.0  0.0   4016  2004 pts/2    S+   14:02   0:00 grep --color=auto 252906
*/