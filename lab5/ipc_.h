#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define ROOMSZ (sizeof(Msg_buf) * 13)
#define SOFASZ (sizeof(Msg_buf) * 4)

void* set_shm(key_t shm_key, int shm_size, int shm_flg);
int set_msq(key_t key, int msq_flag);
int set_sem(key_t sem_key, int sem_val, int sem_flg);

int down(int sem_id);
int up(int sem_id);

void clear_msq(int msq_id);

// 信号量控制用的共同体
typedef union semuns
{
    int val;
} Sem_uns;

// 消息缓冲区
typedef struct msgbuf
{   
    long mtype;
    int  data;
} Msg_buf;

extern key_t buff_key;
extern int* buff_ptr;
extern int shm_flg;

extern key_t mtx_key;
extern int mtx_sem;
extern int sem_flg;
extern int sem_val;

extern int room_flg;
extern key_t room_key;
extern int room_id;

extern int sofa_flg;
extern key_t sofa_key;
extern int sofa_id;

