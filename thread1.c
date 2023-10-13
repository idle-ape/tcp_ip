// 线程的创建和执行
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void *thread_main(void *arg);

int main(int argc, char *argv[])
{
    pthread_t t_id;
    int thread_param = 5;

    if (pthread_create(&t_id, NULL, thread_main, (void *)&thread_param) != 0)
    {
        puts("pthread_create() error");
        return -1;
    }
    // 这里等待10s是为了等待线程执行结束，如果这里小于5，那线程执行不到5次！
    // 所以得用pthread_join等待线程结束，因为进程并不知道线程什么时候结束。
    // sleep(10);
    sleep(2);
    puts("end of main");
    return 0;
}

void *thread_main(void *arg)
{
    int i;
    int cnt = *((int *)arg);
    for (size_t i = 0; i < cnt; i++)
    {
        sleep(1);
        puts("thread running");
    }
    return NULL;
}