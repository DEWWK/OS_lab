#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define max_size 100

void task1(int*);
void task2(int*);
void task3(int*);

int pipe1[2], pipe2[2];
pthread_t thrd1, thrd2, thrd3;
int x, y;

int main(int argc, char *arg[])
{
    int ret;
    int num1, num2, num3;
    printf("please enter x: "); scanf("%d", &x);
    printf("please enter y: "); scanf("%d", &y);

    if (pipe(pipe1) < 0)
    {
        perror("pipe1 not create");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe2) < 0)
    {
        perror("pipe2 not create");
        exit(EXIT_FAILURE);
    }

    num1 = 1;
    ret = pthread_create(&thrd1, NULL, (void*) task1, (void*) &num1);
    if (ret)
    {
        perror("pthread_create: task1");
        exit(EXIT_FAILURE);
    }

    num2 = 2;
    ret = pthread_create(&thrd2, NULL, (void*) task2, (void*) &num2);
    if (ret)
    {
        perror("pthread_create: task2");
        exit(EXIT_FAILURE);
    }

    num3 = 3;
    ret = pthread_create(&thrd3, NULL, (void*) task3, (void*) &num3);
    if (ret)
    {
        perror("pthread_create: task3");
        exit(EXIT_FAILURE);
    }

    pthread_join(thrd1, NULL);
    pthread_join(thrd3, NULL);
    pthread_join(thrd3, NULL);

    exit(EXIT_SUCCESS);
}

void task1(int *num)
{
    int fx[max_size] = {0};

    fx[1] = 1; 
    for (int i = 2; i <= x; i ++ )
    {
        fx[i] = fx[i - 1] * i;
    }

    printf("thread%d: f(x) = %d\n", *num, fx[x]);
    write(pipe1[1], &fx[x], sizeof(int));

    close(pipe1[1]);
}

void task2(int *num)
{
    int fy[max_size] = {0};

    fy[1] = fy[2] = 1;
    for (int i = 3; i <= y; i ++ )
    {
        fy[i] = fy[i - 1] + fy[i - 2];
    }

    printf("thread%d: f(y) = %d\n", *num, fy[y]);
    write(pipe2[1], &fy[y], sizeof(int));
    
    close(pipe2[1]);
}

void task3(int *num)
{
    int fx = 0, fy = 0, fxy = 0;

    read(pipe1[0], &fx, sizeof(int));
    read(pipe2[0], &fy, sizeof(int));
    fxy = fx + fy;

    printf("thread%d: f(x, y) = %d\n", *num, fxy);

    close(pipe1[0]);
    close(pipe2[0]);
}