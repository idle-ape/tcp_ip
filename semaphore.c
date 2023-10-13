// 通过信号量实现线程安全
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUF_SIZE 10

void *put_global_var(void *arg);
void *thread_sum_main(void *arg);

int var = 0;
int sum = 0;
char buf[BUF_SIZE];

// 这里需要两个信号量：
//  一个信号量是为了防止在调用 thread_sum_main 函数的线程还未取走数据的情况下，调用 put_global_var 函数的线程覆盖原值
//  一个信号量是为了防止在调用 put_global_var 函数的线程写入新值前，调用 thread_sum_main 函数的线程取走旧的值
sem_t t1, t2;

int main(int argc, char *argv)
{
    pthread_t tid1, tid2;

    sem_init(&t1, 0, 0);
    sem_init(&t2, 0, 1);

    pthread_create(&tid1, NULL, put_global_var, NULL);
    pthread_create(&tid2, NULL, thread_sum_main, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    printf("sum: %d\n", sum);
    sem_destroy(&t1);
    sem_destroy(&t2);
    return 0;
}

void *put_global_var(void *arg)
{
    for (size_t i = 0; i < 5; i++)
    {
        fputs("Please input a number: ", stdout);
        fflush(stdout);
        int read_len = read(0, buf, BUF_SIZE);
        if (read_len > 0)
        {
            sem_wait(&t2); // t2 -1
            buf[read_len] = 0;
            var = atoi(buf);
            // printf("var: %d\n", var);
            sem_post(&t1); // t1 + 1，56行不阻塞
        }
    }
    return NULL;
}

void *thread_sum_main(void *arg)
{
    for (size_t i = 0; i < 5; i++)
    {
        sem_wait(&t1); // t1 初始值是0，需要等到46行执行后，这里才不阻塞
        sum += var;
        // printf("sum: %d\n", sum);
        sem_post(&t2);
    }
    return NULL;
}