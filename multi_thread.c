// 利用多线程（工作线程）模型进行累加
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

void *thread_summation(void *arg);

int sum = 0;

int main(int argc, char *argv)
{
    pthread_t tid1, tid2;
    int range1[] = {1, 5};
    int range2[] = {6, 10};

    pthread_create(&tid1, NULL, thread_summation, (void *)range1);
    pthread_create(&tid2, NULL, thread_summation, (void *)range2);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    printf("sum: %d\n", sum);
    return 0;
}

void *thread_summation(void *arg)
{
    int start = ((int *)arg)[0];
    int end = ((int *)arg)[1];

    while (start <= end)
    {
        sum += start; // 两个线程直接访问了全局变量！！！！说明该方法非线程安全！
        start++;
    }
    return NULL;
}