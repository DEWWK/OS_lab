#include "dp1.h"

int Sema::down()
{   
    struct sembuf buf;
    buf.sem_op = -1; // 执行P操作7
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
    int sem_key = 201;
    int sem_val;
    int sem_id;
    Sema* sema;

    rate = r;

    // 创建管程锁，全局唯一
    sem_val = 1;
    sem_id = set_sem(sem_key ++ , sem_val, ipc_flg);
    sema = new Sema(sem_id);
    lock = new Lock(sema);

    for (int i = 0; i < NUM; i ++ )
    {   
        // 创建所有车的状态变量
        if ((state_n[i] = (char*)set_shm(shm_key ++ , shm_size, ipc_flg)) == NULL)
        {
            perror("Share memory create error");
            exit(EXIT_FAILURE);
        }
        *state_n[i] = resting;
        if ((state_s[i] = (char*)set_shm(shm_key ++ , shm_size, ipc_flg)) == NULL)
        {
            perror("Share memory create error");
            exit(EXIT_FAILURE);
        }
        *state_s[i] = resting;

        // 建立初值为0的用于条件变量的信号量
        if ((sem_id = set_sem(sem_key ++ , 0, ipc_flg)) < 0)
        {
            perror("Semaphor create error");
            exit(EXIT_FAILURE);
        }
        sema = new Sema(sem_id);
        self_n[i] = new Condition(state_n, state_s, sema);
        self_s[i] = new Condition(state_n, state_s, sema);
    }
}

// 获取筷子操作
void dp::pickup_n(int i)
{   
    std::cout << "n" << i + 1 << "想要拿锁发车" << std::endl;
    *state_n[i] = wantingLock;
    // 设置拿锁的优先级，编号越小的拿锁的优先级较高
    while (1)
    {
        bool can_get_lock = true;
        for (int j = 0; j < i; j ++ )
        {
            if (*state_n[j] == wantingLock)
                can_get_lock = false;
        }
        if (can_get_lock) break;
    }
    lock->close_lock(); // 前提是先获得锁
    std::cout << "n" << i + 1 << "获得锁 " << std::endl;
    
    *state_n[i] = waiting;
    
    bool need_wait = false;
    for (int j = 0; j < NUM; j ++ )
    {
        if (*state_s[j] == waiting)
        {   
            need_wait = true;
            break;
        }
    }
    if (need_wait)
    {   
        std::cout << "n" << i + 1 << "先让南车运行, 暂时释放锁" << std::endl;
        lock->open_lock();
        *state_n[i] = wantingLock;
        sleep(rate);
        while (1)
        {
            bool can_get_lock = true;
            for (int j = 0; j < i; j ++ )
            {
                if (*state_n[j] == wantingLock)
                    can_get_lock = false;
            }
            if (can_get_lock) break;
        }
        lock->close_lock();
        std::cout << "n" << i + 1 << "重新获得锁" << std::endl;
        *state_n[i] = waiting;
    }

    self_n[i]->Wait_n(lock, i); // 测试是否能拿到两只筷子
    std::cout << "n" << i + 1 << "发车\n";
    std::cout << "n" << i + 1 << "释放锁 " << std::endl;
    lock->open_lock(); // 释放锁
    sleep(rate); // 开rate秒
}

void dp::pickup_s(int i)
{   
    std::cout << "s" << i + 1 << "想要拿锁发车" << std::endl;
    *state_s[i] = wantingLock;
    // 设置拿锁的优先级，编号越小的拿锁的优先级较高
    while (1)
    {
        bool can_get_lock = true;
        for (int j = 0; j < i; j ++ )
        {
            if (*state_s[j] == wantingLock)
                can_get_lock = false;
        }
        if (can_get_lock) break;
    }
    lock->close_lock();
    std::cout << "s" << i + 1 << "获得锁 " << std::endl;
    *state_s[i] = waiting; // 尝试拿起筷子的时候就是饥饿态，不然吃不了

    bool need_wait = false;
    for (int j = 0; j < NUM; j ++ )
    {
        if (*state_n[j] == waiting)
        {   
            need_wait = true;
            std::cout << "s" << i + 1 << "先叫醒已经在等的n" << j + 1 << std::endl;
            self_n[j]->Signal_n(j);
        }
    }
    if (need_wait)
    {   
        std::cout << "s" << i + 1 << "先让北车运行, 暂时释放锁" << std::endl;
        lock->open_lock();
        *state_s[i] = wantingLock;
        sleep(rate);
        while (1)
        {
            bool can_get_lock = true;
            for (int j = 0; j < i; j ++ )
            {
                if (*state_s[j] == wantingLock)
                    can_get_lock = false;
            }
            if (can_get_lock) break;
        }
        lock->close_lock();
        std::cout << "s" << i + 1 << "重新获得锁" << std::endl;
        *state_s[i] = waiting;
    }

    self_s[i]->Wait_s(lock, i); // 测试是否能拿到两只筷子
    std::cout << "s" << i + 1 << "发车\n";
    std::cout << "s" << i + 1 << "释放锁 " << std::endl;
    lock->open_lock(); // 释放锁
    sleep(rate); // 开rate秒
}

// 等待操作，先尝试吃，如果吃不了就进入阻塞
void Condition::Wait_n(Lock *lock, int i)
{   
    bool flag = true;
    for (int j = 0; j < NUM; j ++ )
    {
        if (*state_s[j] == running)
        {   
            flag = false;
            break;
        }
    }
    
    if (flag) 
    {
        std::cout << "没有南车在跑, n" << i + 1 << "可以发车" << std::endl;
        *state_n[i] = running;
    }
    // 否则进入阻塞
    else
    {
        // 在进入阻塞之前先释放锁
        // 这保证了线程在等待时不会持有锁
        // 如果在睡眠的时候拿着锁不放，万一其它进程需要锁就会无法获得
        std::cout << "有南车在跑, n" << i + 1 << "被阻塞, 暂时释放锁" << std::endl;
        lock->open_lock();
        sema->down(); // 对条件变量的信号量进行down操作，用于模拟进入阻塞
        
        // 当被唤醒时，重新获得锁
        // 同样要确保获得锁的优先级
        while (1)
        {
            bool can_get_lock = true;
            for (int j = 0; j < i; j ++ )
            {
                if (*state_n[j] == wantingLock)
                    can_get_lock = false;
            }
            if (can_get_lock) break;
        }
        lock->close_lock();
        std::cout << "n" << i + 1 << "重新获得锁" << std::endl;
        *state_n[i] = running;
    }
}

void Condition::Wait_s(Lock *lock, int i)
{   
    bool flag = true;
    for (int j = 0; j < NUM; j ++ )
    {
        if (*state_n[j] == running)
        {   
            flag = false;
            break;
        }
    }
    
    if (flag) 
    {
        std::cout << "没有北车在跑, s" << i + 1 << "可以发车" << std::endl;
        *state_s[i] = running;
    }
    // 否则进入阻塞
    else
    {
        // 在进入阻塞之前先释放锁
        // 这保证了线程在等待时不会持有锁
        // 如果在睡眠的时候拿着锁不放，万一其它进程需要锁就会无法获得
        std::cout << "有北车在跑, s" << i + 1 << "被阻塞, 暂时释放锁" << std::endl;
        lock->open_lock();
        sema->down(); // 对条件变量的信号量进行down操作，用于模拟进入阻塞
        
        // 当被唤醒时，重新获得锁
        while (1)
        {
            bool can_get_lock = true;
            for (int j = 0; j < i; j ++ )
            {
                if (*state_s[j] == wantingLock)
                    can_get_lock = false;
            }
            if (can_get_lock) break;
        }
        lock->close_lock();
        std::cout << "s" << i + 1 << "重新获得锁" << std::endl;
        *state_s[i] = running;
    }
}

// 唤醒操作
void Condition::Signal_n(int i)
{
    sema->up();
    std::cout << "n" << i + 1 << "从阻塞中被唤醒" << std::endl;
    *state_n[i] = wantingLock;
}

void Condition::Signal_s(int i)
{
    sema->up();
    std::cout << "s" << i + 1 << "从阻塞中被唤醒" << std::endl;
    *state_s[i] = wantingLock;
}

void dp::putdown_n(int i)
{
    std::cout << "n" << i + 1 << "想要拿锁结束" << std::endl;
    *state_n[i] = wantingLock;
    // 设置拿锁的优先级，编号越小的拿锁的优先级较高
    while (1)
    {
        bool can_get_lock = true;
        for (int j = 0; j < i; j ++ )
        {
            if (*state_n[j] == wantingLock)
                can_get_lock = false;
        }
        if (can_get_lock) break;
    }
    lock->close_lock(); // 获得锁
    std::cout << "n" << i + 1 << "获得锁" << std::endl;

    *state_n[i] = resting; // 进入休息

    std::cout << "n" << i + 1 << "结束运行" << std::endl;

    // 检查自己后面的车还有没有在运行
    int flag = true;
    for (int j = i + 1; j < NUM; j ++ )
    {
        if (*state_n[j] == running)
        {
            flag = false;
            break;
        }
    }
        // 尝试唤醒
    if (flag)
    {
        std::cout << "n" << i + 1 << "后面无车运行, 尝试唤醒南车" << std::endl;
        for (int j = 0; j < NUM; j ++ )
        {   
            if (*state_s[j] == waiting)
                self_s[j]->Signal_s(j);
        }
    }
    std::cout << "n" << i + 1 << "释放锁" << std::endl;
    lock->open_lock(); // 释放锁
}

void dp::putdown_s(int i)
{
    std::cout << "s" << i + 1 << "想要拿锁结束" << std::endl;
    *state_s[i] = wantingLock;
    // 设置拿锁的优先级，编号越小的拿锁的优先级较高
    while (1)
    {
        bool can_get_lock = true;
        for (int j = 0; j < i; j ++ )
        {
            if (*state_s[j] == wantingLock)
                can_get_lock = false;
        }
        if (can_get_lock) break;
    }
    lock->close_lock(); // 获得锁
    std::cout << "s" << i + 1 << "获得锁" << std::endl;

    *state_s[i] = resting; // 进入休息

    std::cout << "s" << i + 1 << "结束运行" << std::endl;

    // 检查自己后面的车还有没有在运行
    int flag = true;
    for (int j = i + 1; j < NUM; j ++ )
    {
        if (*state_s[j] == running)
        {
            flag = false;
            break;
        }
    }
    // 尝试唤醒
    if (flag)
    {
        std::cout << "s" << i + 1 << "后面无车运行, 尝试唤醒北车" << std::endl;
        for (int j = 0; j < NUM; j ++ )
        {   
            if (*state_n[j] == waiting)
                self_n[j]->Signal_n(j);
        }
    }
    std::cout << "s" << i + 1 << "释放锁" << std::endl;
    lock->open_lock(); // 释放锁
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
    int nid[NUM], sid[NUM];
    int cnt_n = - 1; int cnt_s = -1;

    int rate;
    rate = (argc > 1) ? atoi(argv[1]) : 3;

    tdp = new dp(rate); // 建立一个哲学家就餐的管程对象
    
    int T = NUM * 2;

    while (T -- )
    {
        std::string f;
        std::cin >> f;
        if (f == "n")
        {   
            ++ cnt_n;
            if (cnt_n == NUM)
                std::cout << "北车已发完" << std::endl;
            nid[cnt_n] = fork();
            if (nid[cnt_n] == 0)
            {
                tdp->pickup_n(cnt_n);
                tdp->putdown_n(cnt_n);
                exit(0);
            }
        }
        else if (f == "s")
        {   
            ++ cnt_s;
            if (cnt_s == NUM)
                std::cout << "南车已发完" << std::endl;
            sid[cnt_s] = fork();
            if (sid[cnt_s] == 0)
            {  
                tdp->pickup_s(cnt_s);
                tdp->putdown_s(cnt_s);
                exit(0);
            }
        }
    }

    return 0;
}
