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

    sofa_flg = IPC_CREAT | 0644;
    sofa_key = 201;
    sofa_id = set_msq(sofa_key, sofa_flg);
    buf.msg_perm.uid = getuid();  // 将消息队列的所有者设为当前用户
    buf.msg_perm.gid = getgid();  // 设置组ID为当前用户组
    buf.msg_perm.mode = 0644;
    buf.msg_qbytes = SOFASZ;
    if (msgctl(sofa_id, IPC_SET, &buf) == -1)
    {
        perror("msgctl IPC_SET failed");
        exit(EXIT_FAILURE);
    }
    clear_msq(sofa_id);

    while (1)
    {
        msgrcv(room_id, &msg_arg, sizeof(msg_arg), 1, 0);
        printf("顾客%d离开休息室\n", msg_arg.data);
        msg_arg.mtype = 2;
        msgsnd(sofa_id, &msg_arg, sizeof(msg_arg), 0);
        printf("顾客%d进入沙发\n", msg_arg.data);
    }

    return EXIT_SUCCESS;
}