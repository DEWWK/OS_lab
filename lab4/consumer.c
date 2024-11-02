#include <unistd.h>
#include "ipc.h"

int main(int argc, char* argv[])
{
    int rate;
    // 可在命令行第一参数指定一个进程睡眠秒数，以调解进程执行速度
    if (argv[1] != NULL) rate = atoi(argv[1]);
    else rate = 3;

    // 共享内存使用的变量
    buff_key = 101;
    buff_num = 8;
    // 消费者在共享内存中取产品的位置
    cget_key = 103;
    cget_num = 1;
    // 共享内存读写权限
    shm_flg = IPC_CREAT | 0644;
    // 获取共享内存的指针和消费者去产品位置索引的指针
    buff_ptr = (char*)set_shm(buff_key, buff_num, shm_flg);
    cget_ptr = (int*)set_shm(cget_key, cget_num, shm_flg);
    
    prod_key = 201; // 生产者同步信号量键值
    pmtx_key = 202; // 生产者互斥信号量键值
    cons_key = 301; // 消费者同步信号量键值
    cmtx_key = 302; // 消费者互斥信号量键值
    sem_flg = IPC_CREAT | 0644;

    // 生产者同步信号量初值为缓冲区最大可用量
    sem_val = buff_num;
    prod_sem = set_sem(prod_key, sem_val, sem_flg);
    // 消费者初始无产品可取，同步信号量初值设为0
    sem_val = 0;
    cons_sem = set_sem(cons_key, sem_val, sem_flg);
    // 消费者互斥信号量初值为1
    sem_val = 1;
    cmtx_sem = set_sem(cmtx_key, sem_val, sem_flg);

    // 循环执行模拟消费者
    while (1)
    {   
        // 如果无产品消费者阻塞
        down(cons_sem);
        // 如果另一个消费者正在取产品，本消费者阻塞
        down(cmtx_sem);

        // 用读一字符的形式模拟消费者取产品，报告本进程号和获取的字符及读取的位置
        sleep(rate);
        printf("%d consumer get %c from Buffer[%d]\n", 
                getpid(), 
                buff_ptr[*cget_ptr], 
                *cget_ptr);
        // 读取位置循环下移
        *cget_ptr = (*cget_ptr + 1) % buff_num;

        up(cmtx_sem); // 唤醒阻塞的消费者
        up(prod_sem); // 唤醒阻塞的生产者
    }

    return EXIT_SUCCESS;
}