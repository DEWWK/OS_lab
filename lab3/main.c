#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <termio.h>

#define ARG_NUM 10 // 最大参数数量
#define CMD_NUM 30 // 最大历史记录的命令数量
#define CMD_LEN 50 // 命令的最长长度

int argc; // 参数个数
char* argv[ARG_NUM]; // 参数数组，用于传入exec函数
char args[ARG_NUM][CMD_LEN]; // 参数数组
char curCmd[CMD_LEN]; // 当前正在执行的指令
char bckCmd[CMD_LEN]; // 备份命令
char history[CMD_NUM][CMD_LEN]; // 历史记录
int cmdNum = 0; // 用于维护历史数据
int historyIndex = -1; // 当前的历史命令索引

int getCmd();
void parse(char cmd[]);
void run(int argc, char* argv[]);
    int reInCmd();
    int reOutCmd();
    int exReOutCmd();
    int pipeCmd();
    int bkgdCmd();

void enableRawMode();
void disableRawMode();

struct termios orig_termios;

int main()
{
    if (!isatty(fileno(stdin)))
    {
        while (fgets(curCmd, CMD_LEN, stdin) != NULL) 
        {
            // 去掉换行符
            int len = strlen(curCmd);
            if (curCmd[len - 1] == '\n') {
                curCmd[len - 1] = '\0';
            }

            // 解析输入命令
            parse(curCmd);

            // 执行解析后的命令
            run(argc, argv);
        }
    }
    else
    { 
        while (1)
        {   
            // 打印行头
            printf("[Kevin]$ ");
            // 如果没有输入命令就进行下一个循环
            if (getCmd() == 0)
            {
                continue;
            }
            // 将命令插入历史记录
            strcpy(history[cmdNum ++ ], curCmd);
            // 将命令进行备份
            strcpy(bckCmd, curCmd);
            // 如果历史记录达到了最大条数，那就从头开始
            if (cmdNum == CMD_NUM)
            {
                cmdNum = 0;
            }
            historyIndex = -1;
            // 对命令进行解析
            parse(curCmd);
            // 对命令进行执行
            run(argc, argv);
        }
    }
}

// 获取命令并返回命令的长度
int getCmd()
{   
    // 在获取新的命令之前清空原来的缓冲区
    memset(curCmd, 0x00, CMD_LEN);
    memset(bckCmd, 0x00, CMD_LEN);

    int pos = 0;
    char c;

    enableRawMode();

    while (1)
    {
        c = getchar();
        if (c == '\n')
        {
            curCmd[pos] = '\0';
            printf("\n");
            break;
        }
        else if (c == 127)
        {
            if (pos > 0)
            {
                pos -- ;
                printf("\b \b");
            }
        }
        else if (c == 27)
        {   
            char c1 = getchar();
            char c2 = getchar();
            // printf("1%c", seq[1]);
            if (c1 == '[')
            {
                if (c2 == 'A')
                {
                    if (cmdNum > 0 && historyIndex < cmdNum - 1)
                    {
                        historyIndex ++ ;

                        for (int i = 0; i < CMD_LEN; i++) {
                            printf("\b \b");
                        }

                        memset(curCmd, 0x00, CMD_LEN);
                        strcpy(curCmd, history[cmdNum - 1 - historyIndex]);
                        printf("\r[Kevin]$ %s", curCmd);
                        pos = strlen(curCmd);
                        fflush(stdout);
                    }
                }
                else if (c2 = 'B')
                {
                    if (historyIndex > 0)
                    {
                        historyIndex -- ;

                        for (int i = 0; i < CMD_LEN; i++) {
                            printf("\b \b");
                        }

                        memset(curCmd, 0x00, CMD_LEN);
                        strcpy(curCmd, history[cmdNum - 1 - historyIndex]);
                        printf("\r[Kevin]$ %s", curCmd);
                        pos = strlen(curCmd);
                        fflush(stdout);
                    }
                    else if (historyIndex == 0)
                    {
                        historyIndex -- ;
                        memset(curCmd, 0x00, CMD_LEN);
                        printf("\r[Kevin]$ %s", curCmd);
                        pos = 0;
                        fflush(stdout);
                    }
                }
            }
        }
        else
        {
            if (pos < CMD_LEN - 1)
            {
                curCmd[pos ++ ] = c;
                printf("%c", c);
            }
        }
    }
    disableRawMode();
    return pos;
}

// 启用原始模式，捕获每个字符
void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);  // 获取当前终端设置
    atexit(disableRawMode);  // 程序退出时恢复终端模式

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);  // 禁用回显和标准输入
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);  // 设置为原始模式
}

// 恢复终端模式
void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);  // 恢复原来的终端模式
}


// 对命令进行解析
void parse(char cmd[])
{   
    // 对argc, argv, args进行初始化
    argc = 0;
    for (int i = 0; i < ARG_NUM; i ++ )
    {
        argv[i] = NULL;
        for (int j = 0; j < CMD_LEN; j ++ )
        {
            args[i][j] = '\0';
        }
    }
    
    // 备份命令
    strcpy(bckCmd, cmd);

    // 构建args
    int len = strlen(cmd);
    for (int i = 0, j = 0; i < len; i ++ )
    {
        if (cmd[i] != ' ')
        {
            args[argc][j ++ ] = cmd[i];
        }
        else
        {
            argc ++ ;
            j = 0;
        }
    }

    // 构建argv
    argc = 0;
    int flg = 0;
    for (int i = 0; cmd[i] != '\0'; i ++ )
    {
        if (flg == 0 && !isspace(cmd[i]))
        {
            flg = 1;
            argv[argc ++ ] = cmd + i;
        }
        else if (flg == 1 && isspace(cmd[i]))
        {
            flg = 0;
            cmd[i] = '\0';
        }
    }
}

void run(int argc, char* argv[])
{   
    // 识别重定向输出命令
    for (int i = 0; i < ARG_NUM; i ++ )
    {
        if (strcmp(args[i], ">") == 0)
        {
            strcpy(curCmd, bckCmd);
            if (reOutCmd())
            {
                printf(" fail to execute reout command\n");
            }
            return;
        }
    }
    // 识别重定向输入命令
    for (int i = 0; i < ARG_NUM; i ++ )
    {
        if (strcmp(args[i], "<") == 0)
        {
            strcpy(curCmd, bckCmd);
            if (reInCmd())
            {
                printf(" fail to execute rein command\n");
            }
            return;
        }
    }
    // 识别追加命令
    for (int i = 0; i < ARG_NUM; i ++ )
    {
        if (strcmp(args[i], ">>") == 0)
        {
            strcpy(curCmd, bckCmd);
            if (exReOutCmd())
            {
                printf(" fail to execute exreout command\n");
            }
            return;
        }
    }
    // 识别管道命令
    for (int i = 0; i < ARG_NUM; i ++ )
    {
        if (strcmp(args[i], "|") == 0)
        {
            strcpy(curCmd, bckCmd);
            if (pipeCmd())
            {
                printf(" fail to execute pipe command\n");
            }
            return;
        }
    }
    // 识别后台命令
    for (int i = 0; i < ARG_NUM; i ++ )
    {
        if (strcmp(args[i], "&") == 0)
        {
            strcpy(curCmd, bckCmd);
            if (bkgdCmd())
            {
                printf(" fail to execute background command\n");
            }
            return;
        }
    }

    // 如果没有上述的情况，就创建子进程去直接执行
    pid_t pid = fork();
    if (pid == -1)
    {
        printf("fail to create child process");
        return;
    }
    else if (pid == 0)
    {
        execvp(argv[0], argv);
        printf("Error: %s\n", argv[0]);
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        int err = WEXITSTATUS(status);

        if (err)
        {
            printf("Error: %s\n", strerror(err));
        }
    }
}

int reOutCmd()
{
    char outFile[CMD_LEN];
    memset(outFile, 0x00, CMD_LEN);

    for (int i = 0; i < argc; i ++ )
    {
        if (strcmp(args[i], ">") == 0)
        {
            if (i + 1 < argc)
            {
                strcpy(outFile, args[i + 1]);
            }
            else
            {
                printf("don't have output file\n");
                exit(1);
            }
        }
    }

    for (int i = 0; i < strlen(curCmd); i ++ )
    {
        if (curCmd[i] == '>')
        {
            curCmd[i - 1] = '\0';
            curCmd[i] = '\0';
            break;
        }
    }

    parse(curCmd);

    pid_t pid = fork();
    if (pid == -1)
    {
        printf("fail to create child process\n");
        exit(1);
    }
    else if (pid == 0)
    {
        int fd = open(outFile, O_WRONLY|O_TRUNC, 7777);
        if (fd < 0)
        {   
            printf("No such file\n");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        execvp(argv[0], argv);
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        int err = WEXITSTATUS(status);
        if (err)
        {
            printf("Error: %s\n", strerror(err));
        }
    }
}

int reInCmd()
{
    char inFile[CMD_LEN];
    memset(inFile, 0x00, CMD_LEN);

    for (int i = 0; i < argc; i ++ )
    {
        if (strcmp(args[i], "<") == 0)
        {
            strcpy(inFile, args[i + 1]);
        }
        else
        {
            printf("miss input file\n");
            exit(1);
        }
    }

    for (int i = 0; i < strlen(curCmd); i ++ )
    {
        if (curCmd[i] == '<')
        {
            curCmd[i - 1] = '\0';
            curCmd[i] = '\0';
            break;
        }
    }
    
    parse(curCmd);

    pid_t pid = fork();
    if (pid == -1)
    {
        printf("fail to create child process\n");
        exit(1);
    }
    else if (pid == 0)
    {
        int fd = open(inFile, O_RDONLY, 7777);
        if (fd < 0)
        {   
            printf("No such file\n");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        execvp(argv[0], argv);
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        int err = WEXITSTATUS(status);
        if (err)
        {
            printf("Error: %s\n", strerror(err));
        }
    }
}

int exReOutCmd()
{
    char outFile[CMD_LEN];
    memset(outFile, 0x00, CMD_LEN);

    for (int i = 0; i < argc; i ++ )
    {
        if (strcmp(args[i], ">>") == 0)
        {
            if (i + 1 < argc)
            {
                strcpy(outFile, args[i + 1]);
            }
            else
            {
                printf("miss output file\n");
                exit(1);
            }
        }
    }
    
    for (int i = 0; i + 2 < strlen(curCmd); i ++ )
    {
        if (curCmd[i] == '>' && curCmd[i + 1] == '>'
                             && curCmd[i + 2] == ' ')
        {
            curCmd[i - 1] = '\0';
            curCmd[i] = '\0';
            break;
        }
    }

    parse(curCmd);

    pid_t pid = fork();
    if (pid == -1)
    {
        printf("fail to create child process\n");
        exit(1);
    }
    else if (pid == 0)
    {
        int fd = open(outFile, O_WRONLY|O_APPEND|O_CREAT|O_APPEND, 7777);
        if (fd < 0)
        {   
            printf("No such file\n");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        execvp(argv[0], argv);
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        int err = WEXITSTATUS(status);
        if (err)
        {
            printf("Error: %s\n", strerror(err));
        }
    }
}

int pipeCmd()
{   
    int i = 0;
    for (i = 0; i < strlen(curCmd); i ++ )
    {
        if (curCmd[i] == '|')
        {
            break;
        }
    }

    char outCmd[CMD_LEN], inCmd[CMD_LEN];
    memset(outCmd, 0x00, CMD_LEN);
    memset(inCmd, 0x00, CMD_LEN);
    for (int j = 0; j < i - 1; j ++ )
    {
        outCmd[j] = curCmd[j];
    }
    for (int j = 0; i + j + 1 < strlen(curCmd); j ++ )
    {
        inCmd[j] = curCmd[i + 2 + j];
    }

    int pd[2];
    if (pipe(pd) < 0)
    {
        perror("pipe()");
        exit(1);
    }

    pid_t pid1 = fork();
    if (pid1 == -1)
    {
        printf("fail to create child process\n");
        exit(1);
    }
    else if (pid1 == 0)
    {
        close(pd[0]);
        dup2(pd[1], STDOUT_FILENO);
        close(pd[1]);

        parse(outCmd);
        execvp(argv[0], argv);
        exit(1);
    }
    else
    {
        pid_t pid2 = fork();
        if (pid2 == -1)
        {
            printf("fail to create child process\n");
            exit(1);
        }
        else if (pid2 == 0)
        {
            close(pd[1]); // 关闭写端
            dup2(pd[0], STDIN_FILENO); // 将标准输入重定向到管道的读端
            close(pd[0]); // 关闭读端

            parse(inCmd);
            execvp(argv[0], argv); // 执行第二个命令
            exit(1);
        }
        else
        {
            close(pd[0]);
            close(pd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            return 0;
        }
    }
}

int bkgdCmd()
{   
    char backgroudCmd[CMD_LEN];
    memset(backgroudCmd, 0x00, CMD_LEN);
    for (int i = 0; i < strlen(curCmd); i ++ )
    {
        backgroudCmd[i] = curCmd[i];
        if (curCmd[i] == '&')
        {
            backgroudCmd[i] = '\0';
            backgroudCmd[i - 1] = '\0';
            break;
        }
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        printf("fail to create child process\n");
        exit(1);
    }
    else if (pid == 0)
    {
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "r", stdin);
        signal(SIGCHLD, SIG_IGN);
        parse(backgroudCmd);
        execvp(argv[0], argv);
        exit(1);
    }
    else
    {
        exit(0);
    }
}