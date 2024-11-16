#include "ipc_.h"
#include <unistd.h>

int main(int argc, char* argv[])
{
    int i; int id;
    Msg_buf msg_arg;

    id = atoi(argv[1]);

    buff_key = 301;
    shm_flg = IPC_CREAT | 0644;
    buff_ptr = (int*)set_shm(buff_key, sizeof(int), shm_flg);
    *buff_ptr = 0;
    
    mtx_key = 401;
    sem_flg = IPC_CREAT | 0644;
    sem_val = 1;
    mtx_sem = set_sem(mtx_key, sem_val, sem_flg);
    int val = semctl(mtx_sem, 0, GETVAL);

    sofa_flg = IPC_CREAT | 0644;
    sofa_key = 201;
    sofa_id = set_msq(sofa_key, sofa_flg);

    while (1)
    {
        msgrcv(sofa_id, &msg_arg, sizeof(msg_arg), 2, 0);
        printf("理发师%d开始给顾客%d理发\n", id, msg_arg.data);
        sleep(5);

        down(mtx_sem);
        *buff_ptr += 20;
        printf("顾客%d结账成功, 收入总额为%d元\n", msg_arg.data, *buff_ptr);
        up(mtx_sem);
    }

    return EXIT_SUCCESS;
}