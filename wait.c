#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "error_handling.h"

int main(int argc, char *argv[])
{
    int status;
    pid_t pid = fork();
    if (pid == 0)
    {
        sleep(10);
        return 3;
    }
    else
    {
        printf("child pid: %d\n", pid);
        pid = fork();
        if (pid == 0)
        {
            sleep(10);
            exit(7);
        }
        else
        {
            printf("child pid: %d\n", pid);
            wait(&status); // 调用wait函数，子进程通过exit或者return终止时的返回值将保存到status变量，同时子进程被完全销毁
            if (WIFEXITED(status)) // WIFEXITED宏验证子进程是否正常终止，如果正常退出，则调用WEXITSTATUS宏输出子进程的返回值
            {
                printf("child send one: %d\n", WEXITSTATUS(status));
            }

            wait(&status);
            if (WIFEXITED(status))
            {
                printf("child send two: %d\n", WEXITSTATUS(status));
            }
            sleep(30); // sleep 30 sec
        }
    }
    return 0;
}

/*
root@ubuntu1:~# ps -aux  | grep -E '253097|253098'
root      253097  0.0  0.0   2636    88 pts/1    S+   14:24   0:00 ./bin/wait
root      253098  0.0  0.0   2768    96 pts/1    S+   14:24   0:00 ./bin/wait
root      253100  0.0  0.0   4016  2100 pts/2    S+   14:24   0:00 grep --color=auto -E 253097|253098
root@ubuntu1:~#
root@ubuntu1:~#
root@ubuntu1:~# ps -aux  | grep -E '253097|253098'
root      253102  0.0  0.0   4016  2000 pts/2    S+   14:24   0:00 grep --color=auto -E 253097|253098
*/

// ⚠️ 调用wait函数时，如果没有已终止的子进程，那么程序将阻塞直到有紫禁城终止，因此需要谨慎调用该函数