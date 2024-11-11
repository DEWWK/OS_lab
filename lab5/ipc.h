#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define BUFSZ 256
#define MAXVAL 100
#define STRSIZ 8
#define WRITREQUEST 1
#define READREQUEST 2
#define FINISHED 3

void* set_shm(key_t shm_key, int shm_size, int shm_flg);
int set_msq(key_t key, int msq_flag);
int set_sem(key_t sem_key, int sem_val, int sem_flg);

int down(int sem_id);
int up(int sem_id);

// 信号量控制用的共同体
typedef union semuns
{
    int val;
} Sem_uns;

// 消息缓冲区
typedef struct msgbuf
{
    long mtype; // 消息的类型
    int  mid;   // 消息的发送者
    int  data;
} Msg_buf;

extern key_t buff_key;
extern int buff_num;
extern char* buff_ptr;
extern int shm_flg;

extern int quest_flg;
extern key_t quest_key;
extern int quest_id;

extern int respond_flg;
extern key_t respond_key;
extern int respond_id;


