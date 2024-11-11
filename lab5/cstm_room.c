#include "ipc_.h"
#include <unistd.h>

int main(int argc, char* argv[])
{
    int i; int rate;
    Msg_buf msg_arg;
    struct msqid_ds buf;

    if (argv[1] != NULL) rate = atoi(argv[1]);
    else rate = 3;

    room_flg = IPC_CREAT | 0644;
    room_key = 101;
    room_id = set_msq(room_key, room_flg);
    buf.msg_perm.uid = getuid();  // 将消息队列的所有者设为当前用户
    buf.msg_perm.gid = getgid();  // 设置组ID为当前用户组
    buf.msg_perm.mode = 0644;
    buf.msg_qbytes = ROOMSZ;
    if (msgctl(room_id, IPC_SET, &buf) == -1)
    {
        perror("msgctl IPC_SET failed");
        exit(EXIT_FAILURE);
    }
    clear_msq(room_id);

    int cstm_num = 1; // 记录顾客的数量
    msg_arg.mtype = 1; // 设置消息的类型为1
    while (1)
    {
        msg_arg.data = cstm_num;
        // 阻塞式向休息室发送顾客
        msgsnd(room_id, &msg_arg, sizeof(msg_arg), 0);
        printf("顾客%d进入休息室\n", msg_arg.data);

        sleep(rate);
        cstm_num ++ ;
    }

    return EXIT_SUCCESS;
}