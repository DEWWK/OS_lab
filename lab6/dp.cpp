#include "dp.h"

int Sema::down()
{   
    struct sembuf buf;
    buf.sem_op = -1; // 执行P操作
    buf.sem_num = 0; // 想要操作的信号量的编号
    buf.sem_flg = SEM_UNDO; 
    if ((semop(sem_id, &buf, 1)) < 0)
    {
        perror("down error");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int Sema::up()
{   
    Sem_uns arg;
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

// 获得锁
void Lock::close_lock()
{
    sema->down();
}

// 释放锁
void Lock::open_lock()
{
    sema->up();
}

dp::dp(int r)
{
    int ipc_flg = IPC_CREAT | 0644;
    int shm_key = 401;
    int shm_size = 1;
    int sem_key = 301;
    int sem_val;
    int sem_id;
    Sema* sema;

    rate = r;

    // 创建管程锁，全局唯一
    sem_val = 1;
    sem_id = set_sem(sem_key ++ , sem_val, ipc_flg);
    sema = new Sema(sem_id);
    lock = new Lock(sema);

    for (int i = 0; i < 5; i ++ )
    {
        // 为每个哲学家建立一个条件变量和可共享的状态
        // 初始状态都为思考
        if ((state[i] = (char*)set_shm(shm_key ++ , shm_size, ipc_flg)) == NULL)
        {
            perror("Share memory create error");
            exit(EXIT_FAILURE);
        }
        *state[i] = thinking;

        // 为每个哲学家建立初值为0的用于条件变量的信号量
        if ((sem_id = set_sem(sem_key ++ , 0, ipc_flg)) < 0)
        {
            perror("Semaphor create error");
            exit(EXIT_FAILURE);
        }
        sema = new Sema(sem_id);
        self[i] = new Condition(state, sema);
    }
}

// 获取筷子操作
void dp::pickup(int i)
{   
    std::cout << "p" << i + 1 << "想要开始吃" << std::endl;
    lock->close_lock(); // 前提是先获得锁
    std::cout << "p" << i + 1 << "获得锁 " << "lock value: " << lock->sema->getValue() << std::endl;
    
    *state[i] = hungry; // 尝试拿起筷子的时候就是饥饿态，不然吃不了
    self[i]->Wait(lock, i); // 测试是否能拿到两只筷子
    std::cout << "p" << i + 1 << "开始吃\n";
    sleep(rate); // 吃rate秒
    lock->open_lock(); // 释放锁
    std::cout << "p" << i + 1 << "释放锁 " << "lock value: " << lock->sema->getValue() << std::endl;
}

// 等待操作，先尝试吃，如果吃不了就进入阻塞
void Condition::Wait(Lock *lock, int i)
{   
    // 左右邻居不在就餐，状态变成就餐
    if ((*state[(i + 4) % 5] != eating) &&
        (*state[i] == hungry) && 
        (*state[(i + 1) % 5] != eating))
    {   
        std::cout << "p" << i + 1 << "的左右邻居没有在吃, 满足吃的条件" << std::endl;
        *state[i] = eating;
    }
    // 否则进入阻塞
    else
    {
        // 在进入阻塞之前先释放锁
        // 这保证了线程在等待时不会持有锁
        // 如果在睡眠的时候拿着锁不放，万一其它进程需要锁就会无法获得
        lock->open_lock();
        std::cout << "p" << i + 1 << "的左右邻居在吃, 被阻塞, 暂时释放锁 " << "lock value: " << lock->sema->getValue() << std::endl;
        sema->down(); // 对条件变量的信号量进行down操作，用于模拟进入阻塞
        // 当被唤醒时，重新获得锁
        lock->close_lock();
        std::cout << "p" << i + 1 << "重新获得锁 " << "lock value: " << lock->sema->getValue() << std::endl;
    }
}

// 唤醒操作
void Condition::Signal(int i)
{
    if ((*state[(i + 4) % 5] != eating) && 
        (*state[i] == hungry) && 
        (*state[(i + 1) % 5] != eating))
    {   
        // 对条件变量的信号量进行Up操作，用于模拟唤醒
        sema->up();
        std::cout << "p" << i + 1 << "从阻塞中被唤醒" << std::endl;
        *state[i] = eating;
    }
    else
    {
        std::cout << "p" << i + 1 << "不饿, 不需要吃饭" << std::endl;
    }
}

void dp::putdown(int i)
{
    std::cout << "p" << i + 1 << "想要结束吃\n";
    int j;
    lock->close_lock(); // 获得锁
    std::cout << "p" << i + 1 << "获得锁 " << "lock value: " << lock->sema->getValue() << std::endl;

    *state[i] = thinking; // 进入思考态

    std::cout << "p" << i + 1 << "结束吃" << std::endl;

    j = (i + 4) % 5;
    std::cout << "p" << i + 1 << "叫p"  << j + 1 << "吃饭" << std::endl;
    self[j]->Signal(j); // 唤醒左邻居
    j = (i + 1) % 5;
    std::cout << "p" << i + 1 << "叫p"  << j + 1 << "吃饭" << std::endl;
    self[j]->Signal(j); // 唤醒右邻居
    lock->open_lock(); // 释放锁
    std::cout << "p" << i + 1 << "释放锁 " << "lock value: " << lock->sema->getValue() << std::endl;
    
    sleep(rate);
}

int dp::set_sem(key_t sem_key, int sem_val, int sem_flg)
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

void *dp::set_shm(key_t shm_key, int shm_size, int shm_flg)
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

int main(int argc, char* argv[])
{
    dp* tdp; // 哲学家就餐管程对象的指针
    int pid[5];

    int rate;
    rate = (argc > 1) ? atoi(argv[1]) : 3;

    tdp = new dp(rate); // 建立一个哲学家就餐的管程对象
    // std::cout << tdp->lock->sema->getValue() << std::endl;
    
    pid[0] = fork();
    if (pid[0] == 0)
    {   
        // std::cout << tdp->lock->sema->getValue() << std::endl;
        while (1)
        {   
            tdp->pickup(0);
            tdp->putdown(0);
        }
    }

    sleep(1);
    pid[1] = fork();
    if (pid[1] == 0)
    {   
        // std::cout << tdp->lock->sema->getValue() << std::endl;
        while (1)
        {   
            
            tdp->pickup(1);
            tdp->putdown(1);
        }
    }

    sleep(1);
    pid[2] = fork();
    if (pid[2] == 0)
    {   
        // std::cout << tdp->lock->sema->getValue() << std::endl;
        while (1)
        {   
            tdp->pickup(2);
            tdp->putdown(2);
        }
    }

    sleep(1);
    pid[3] = fork();
    if (pid[3] == 0)
    {   
        // std::cout << tdp->lock->sema->getValue() << std::endl;
        while (1)
        {   
            
            tdp->pickup(3);
            tdp->putdown(3);
        }
    }

    sleep(1);
    pid[4] = fork();
    if (pid[4] == 0)
    {   
        // std::cout << tdp->lock->sema->getValue() << std::endl;
        while (1)
        {   
            tdp->pickup(4);
            tdp->putdown(4);
        }
    }

    return 0;
}
