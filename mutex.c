// 通过互斥锁保证线程安全
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define NUM_THREAD 100

void *thread_incr(void *arg);
void *thread_desc(void *arg);

long long num = 0;

pthread_mutex_t mutex;

int main(int argc, char *argv[])
{
    pthread_t tid[NUM_THREAD];

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        puts("pthread_mutex_init() error");
        return -1;
    }

    printf("sizeof long long int: %ld\n", sizeof(long long));
    for (size_t i = 0; i < NUM_THREAD; i++)
    {
        if (i % 2 == 0)
        {
            pthread_create(&tid[i], NULL, thread_incr, NULL);
        }
        else
        {
            pthread_create(&tid[i], NULL, thread_desc, NULL);
        }
    }

    for (size_t i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(tid[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    printf("num: %lld\n", num);
    return 0;
}

void *thread_incr(void *arg)
{
    for (size_t i = 0; i < 500000; i++)
    {
        pthread_mutex_lock(&mutex);
        num += 1;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *thread_desc(void *arg)
{
    for (size_t i = 0; i < 500000; i++)
    {
        pthread_mutex_lock(&mutex);
        num -= 1;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

/*
root@ubuntu1:~/clang# ./bin/mutex
sizeof long long int: 8
num: 0
*/