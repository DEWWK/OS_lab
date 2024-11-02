#ifndef IPC_H
#define IPC_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define BUFSZ 256

// 创建或获取共享内存，返回共享内存的地址指针
void* set_shm(key_t shm_key, int shm_size, int shm_flg);
// 创建或获取一个信号量
int set_sem(key_t sem_key, int sem_val, int sem_flg);

// 实现信号量的PV操作
int down(int sem_id);
int up(int sem_id);

// 用于初始化信号量的联合体
typedef union semuns
{
    int val; // 信号量的值
} Sem_uns;
// 消息结构体
typedef struct msgbuf
{
    long mtype; // 消息类型
    char mtext[1]; // 消息内容
} Msg_buf;

extern key_t buff_key; // 共享内存的键值
extern int buff_num; // 共享内存大小
extern char *buff_ptr; // 共享内存指针

// 生产者放产品的位置和信息
extern key_t pput_key;
extern int pput_num;
extern int* pput_ptr;

// 消费者取产品的位置和信息
extern key_t cget_key;
extern int cget_num;
extern int* cget_ptr;

// 生产者信号量信息
extern key_t prod_key;
extern key_t pmtx_key;
extern int prod_sem;
extern int pmtx_sem;

// 消费者信号量信息
extern key_t cons_key;
extern key_t cmtx_key;
extern int cons_sem;
extern int cmtx_sem;

// 信号量初始值
extern int sem_val;
// 用于信号量和共享内存的创建标志
extern int sem_flg;
extern int shm_flg;

extern char* filepath;

#endif