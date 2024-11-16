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

typedef union semuns
{
    int val;
} Sem_uns;

enum State
{
    thinking, hungry, eating
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
    Condition(char* st[], Sema* sm) : 
        state(st), sema(sm) {}
    ~Condition() {}
    void Wait(Lock* lock, int i); // 阻塞操作
    void Signal(int i); // 唤醒操作
  private:
    Sema* sema; // 条件变量的信号量，用于实现模拟等待和唤醒操作
    char** state; // 哲学家当前状态
};

// 哲学家管程
class dp
{
  public:
    dp(int r);
    ~dp() {}
    void pickup(int i); // 获取筷子
    void putdown(int i); // 放下筷子
    // 建立一个具有n个信号量的信号量
    int set_sem(key_t sem_key, int sem_val, int sem_flg);
    // 建立一个具有n个字节的共享内存区
    void* set_shm(key_t shm_key, int shm_size, int shm_flg);
    int rate; // 控制执行速度
    Lock* lock; // 控制互斥进入管程的锁
    char* state[5]; // 5个哲学家当前的状态
    Condition* self[5]; // 控制5个哲学家状态的条件变量
};