// 验证非线程安全
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define NUM_THREAD 100

void *thread_incr(void *arg);
void *thread_desc(void *arg);

long long num = 0;

int main(int argc, char *argv[])
{
    pthread_t tid[NUM_THREAD];

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
    printf("num: %lld\n", num);
    return 0;
}

void *thread_incr(void *arg)
{
    for (size_t i = 0; i < 500000; i++)
    {
        num += 1;
    }
    return NULL;
}

void *thread_desc(void *arg)
{
    for (size_t i = 0; i < 500000; i++)
    {
        num -= 1;
    }
    return NULL;
}

/*
root@ubuntu1:~/clang# gcc -o ./bin/multi_thread2 multi_thread2.c
root@ubuntu1:~/clang#
root@ubuntu1:~/clang#
root@ubuntu1:~/clang# ./bin/multi_thread2
sizeof long long int: 8
num: 9355
root@ubuntu1:~/clang# ./bin/multi_thread2
sizeof long long int: 8
num: 967734
root@ubuntu1:~/clang# ./bin/multi_thread2
sizeof long long int: 8
num: -639188
root@ubuntu1:~/clang# ./bin/multi_thread2
sizeof long long int: 8
num: 950263
root@ubuntu1:~/clang# ./bin/multi_thread2
sizeof long long int: 8
num: 494621

可以看到每次执行的结果都不一样！！！！！这就是非线程安全的后果！！！
*/