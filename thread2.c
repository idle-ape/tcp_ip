// 通过 pthread_join 函数等待线程结束并获取线程返回值
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

void *thread_main(void *arg);

int main(int argc, char *argv[])
{
    pthread_t t_id;
    int thread_param = 5;
    void *thr_ret;

    if (pthread_create(&t_id, NULL, thread_main, (void *)&thread_param) != 0)
    {
        puts("pthread_create() error");
        return -1;
    }

    if (pthread_join(t_id, &thr_ret) != 0)
    {
        puts("pthread_join() error");
        return -1;
    }

    printf("thread return message: %s\n", (char *)thr_ret);
    free(thr_ret);
    return 0;
}

void *thread_main(void *arg)
{
    int i;
    int cnt = *((int *)arg);
    char *msg = (char *)malloc(sizeof(char) * 50);
    strcpy(msg, "Hello, I'am thread~\n");
    for (size_t i = 0; i < cnt; i++)
    {
        sleep(1);
        puts("thread running");
    }
    return (void *)msg;
}