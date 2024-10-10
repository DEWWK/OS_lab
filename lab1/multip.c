#include "pctl.h"

int main(int argc, char *argv[])
{
    int pid_ls, pid_ps;
    int status;

    char *args_ls[] = {"/bin/ls", NULL};
    char *args_ps[] = {"/bin/ps", NULL};

    signal(SIGINT, (__sighandler_t)sigcat);
    perror("SIGINT");

    sleep(3);
    pid_ls = fork();
    if (pid_ls < 0)
    {
        printf("Create ls Process fail!\n");
        exit(EXIT_FAILURE);
    }
    if (pid_ls == 0)
    {
        printf("I am ls subprocess, I am Child process %d\nMy father is %d\n",
               getpid(), getppid());
        pause();
        printf("ls subprocess will running");
        status = execve(args_ls[0], args_ls, NULL);
    }
    else
    {
        printf("\nI am Parent process %d\n", getpid());
        sleep(3);

        pid_ps = fork();
        if (pid_ps < 0)
        {
            printf("Create ps Process fail!\n");
            exit(EXIT_FAILURE);
        }
        if (pid_ps == 0)
        {
            printf("I am ps subprocess, I am Child process %d\nMy father is %d\n",
                   getpid(), getppid());
            status = execve(args_ps[0], args_ps, NULL);
        }
        else 
        {
            sleep(5);
            if (kill(pid_ls, SIGINT) >= 0)
            {
                printf("Wake up ls child %d\n", pid_ls);
            }
            waitpid(pid_ls, &status, 0);
            waitpid(pid_ps, &status, 0);
            printf("\nFather Over\n");
        }
    }
    return EXIT_SUCCESS;
}