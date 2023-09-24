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
        sleep(12);
        return 3;
    }
    else
    {
        while (!waitpid(-1, &status, WNOHANG))
        {
            sleep(3);
            puts("sleep 3sec.");
        }
        
        if (WIFEXITED(status))
        {
            printf("child send %d\n", WEXITSTATUS(status));
        }
    }
    return 0;
}

/*
root@ubuntu1:~/clang# ./bin/waitpid
sleep 3sec.
sleep 3sec.
sleep 3sec.
sleep 3sec.
child send 3
*/

// ⚠️ 调用waitpid函数，因为不会阻塞，所以每隔3sec就会打印一行，直到子进程返回。