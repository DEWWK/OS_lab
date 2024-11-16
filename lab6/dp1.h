#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define NUM 10 // 车的数量

typedef union semuns
{
    int val;
} Sem_uns;

enum State
{
    resting, waiting, running, wantingLock
};

// 信号量
class Sema
{
  public:
    Sema(int id) : sem_id(id) {}
    ~Sema() {}
    int down();
    int up();
    int getValue()
    {
        return semctl(sem_id, 0, GETVAL);
    }
  private:
    int sem_id;
};

class Lock
{
  public:
    Lock(Sema *s) : sema(s) {}
    ~Lock() {}
    void close_lock();
    void open_lock();
    Sema *sema;
};

class Condition
{
  public:
    Condition(char* st_n[], char* st_s[], Sema* sm) : 
        state_n(st_n), state_s(st_s), sema(sm) {}
    ~Condition() {}
    void Wait_n(Lock* lock, int i); // 阻塞操作
    void Wait_s(Lock* lock, int i); // 阻塞操作
    void Signal_n(int i); // 唤醒操作
    void Signal_s(int i); // 唤醒操作
  private:
    Sema* sema; // 条件变量的信号量，用于实现模拟等待和唤醒操作
    char** state_n; // 北车状态
    char** state_s; // 南车状态 
};

class dp
{
  public:
    dp(int r);
    ~dp() {}
    void pickup_n(int i); // 北车
    void pickup_s(int i); // 南车
    void putdown_n(int i); // 放下筷子
    void putdown_s(int i); // 放下筷子
    // 建立信号量
    int set_sem(key_t sem_key, int sem_val, int sem_flg);
    // 建立共享内存
    void* set_shm(key_t shm_key, int shm_size, int shm_flg);
    int rate; // 控制执行速度
    Lock* lock; // 控制互斥进入管程的锁
    char* state_n[NUM];
    char* state_s[NUM];
    Condition* self_n[NUM];
    Condition* self_s[NUM];
};