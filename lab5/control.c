#include "ipc.h"

// 每次启动时清空消息队列
void clear_msq(int msq_id)
{
    Msg_buf msg_arg;
    while (msgrcv(msq_id, &msg_arg, sizeof(msg_arg), 0, IPC_NOWAIT) != -1);
}

int main(int argc, char* argv[])
{   
    int i;
    int rate;
    int w_mid;
    int count = MAXVAL;
    Msg_buf msg_arg;
    struct msqid_ds msg_inf;

    buff_key = 101;
    buff_num = STRSIZ + 1;
    shm_flg = IPC_CREAT | 0644;
    buff_ptr = (char*)set_shm(buff_key, buff_num, shm_flg);
    for (i = 0; i < STRSIZ; i ++ )
    {
        buff_ptr[i] = 'A';
    }
    buff_ptr[i] = '\0';

    // 创建一条请求消息队列
    quest_flg = IPC_CREAT | 0644;
    quest_key = 201;
    quest_id = set_msq(quest_key, quest_flg);
    clear_msq(quest_id);

    // 创建一条响应消息队列
    respond_flg = IPC_CREAT | 0644;
    respond_key = 202;
    respond_id = set_msq(respond_key, respond_flg);
    clear_msq(respond_id);

    printf("Wait quest\n");

    while (1)
    {   
        if (count == MAXVAL)
        {
            quest_flg = IPC_NOWAIT;
            if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), WRITREQUEST, quest_flg) >= 0)
            {
                count = 0;
                msg_arg.mtype = msg_arg.mid;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("%d quest write\n", msg_arg.mid);
            }
            if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), READREQUEST, quest_flg) >= 0)
            {
                count -- ;
                msg_arg.mtype = msg_arg.mid;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("%d quest read, count=%d\n", msg_arg.mid, count);
            }
        }
        // 如果count > 0代表写者没有在写，所以接收的FINISHED信号必是读者的
        else if (count > 0)
        {
            quest_flg = IPC_NOWAIT; // 以非阻塞方式请求消息

            if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), FINISHED, quest_flg) >= 0)
            {
                count ++ ;
                printf("%d reader finished %d, count=%d\n", msg_arg.mid, msg_arg.data, count);
            }
            else if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), READREQUEST, quest_flg) >= 0)
            {
                count -- ;
                // 方便通信，让请求知道哪个是属于自己的响应
                msg_arg.mtype = msg_arg.mid;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("%d quest read %d, count=%d\n", msg_arg.mid, msg_arg.data, count);
            }
        }
        // 如果count == 0说明写者在写，这时地FINISH信号必是写者的
        // 所以要先阻塞式地接收写者地结束信号，再去理会读请求信号
        else if (count == 0)
        {
            msgrcv(quest_id, &msg_arg, sizeof(msg_arg), FINISHED, 0);
            count = MAXVAL;
            printf("%d write finished\n", msg_arg.mid);
        }
    }

    return EXIT_SUCCESS;
}