#include "ipc.h"
#include <string.h>

key_t buff_key; 
int buff_num; 
char *buff_ptr;
 
key_t pput_key;
int pput_num;
int* pput_ptr;

key_t cget_key;
int cget_num;
int* cget_ptr;

key_t prod_key;
key_t pmtx_key;
int prod_sem;
int pmtx_sem;

key_t cons_key;
key_t cmtx_key;
int cons_sem;
int cmtx_sem;

int sem_val;
int sem_flg;
int shm_flg;

// P操作
int down(int sem_id)
{   
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0)
    {
        perror("down error");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int up(int sem_id)
{
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0)
    {
        perror("up error");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int set_sem(key_t sem_key, int sem_val, int sem_flg)
{
    int sem_id;
    Sem_uns sem_arg;
    // 判断sem_key标识的信号量集是否已经建立
    sem_arg.val = sem_val;

    // 创建或获取信号量集
    sem_id = semget(sem_key, 1, sem_flg);
    if (sem_id < 0) {
        perror("semaphore create error");
        exit(EXIT_FAILURE);
    }

    // 设置信号量初始值
    if (semctl(sem_id, 0, SETVAL, sem_arg) < 0) {
        perror("semaphore set error");
        exit(EXIT_FAILURE);
    }

    return sem_id;
}

void* set_shm(key_t shm_key, int shm_size, int shm_flg)
{
    int shm_id;
    void* shm_buf;
    
    // 获取或创建共享内存
    shm_id = shmget(shm_key, shm_size, shm_flg);
    if (shm_id < 0) {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    // 将共享内存附加到进程地址空间
    shm_buf = shmat(shm_id, NULL, 0);
    if (shm_buf == (void*)-1) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }

    // 如果是创建新的共享内存，则将其初始化为0
    struct shmid_ds shm_stat;
    if (shmctl(shm_id, IPC_STAT, &shm_stat) == -1) {
        perror("shmctl error");
        exit(EXIT_FAILURE);
    }

    if (shm_stat.shm_nattch == 1) {  // 检查当前是首次附加
        memset(shm_buf, 0, shm_size);   // 初始化共享内存为0
    }

    return shm_buf;
}
