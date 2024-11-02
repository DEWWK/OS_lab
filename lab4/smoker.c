#include <unistd.h>
#include "ipc.h"

int main(int argc, char* argv[])
{
    int type = atoi(argv[1]);

    key_t cigar_buf_key = 101;
    key_t paper_buf_key = 201;
    key_t glu_buf_key   = 301;
    buff_num = 8;
    key_t cigar_get_key = 102;
    key_t paper_get_key = 202;
    key_t glu_get_key   = 302;
    cget_num = 1;

    shm_flg = IPC_CREAT | 0644;

    char* cigar_buf_ptr = (char*)set_shm(cigar_buf_key, buff_num, shm_flg);
    char* paper_buf_ptr = (char*)set_shm(paper_buf_key, buff_num, shm_flg);
    char* glu_buf_ptr   = (char*)set_shm(glu_buf_key,   buff_num, shm_flg);

    int* cigar_get_ptr = (int*)set_shm(cigar_get_key, cget_num, shm_flg);
    int* paper_get_ptr = (int*)set_shm(paper_get_key, cget_num, shm_flg);
    int* glu_get_ptr   = (int*)set_shm(glu_get_key,   cget_num, shm_flg);

    key_t cigar_cons_key = 103;
    key_t cigar_prod_key = 104;
    key_t cigar_cons_mtx_key = 105;

    key_t paper_cons_key = 203;
    key_t paper_prod_key = 204;
    key_t paper_cons_mtx_key = 205;

    key_t glu_cons_key = 303;
    key_t glu_prod_key = 304;
    key_t glu_cons_mtx_key = 305;

    sem_flg = IPC_CREAT | 0644;

    int cigar_cons_sem = set_sem(cigar_cons_key, 0, sem_flg);
    int paper_cons_sem = set_sem(paper_cons_key, 0, sem_flg);
    int glu_cons_sem = set_sem(glu_cons_key, 0, sem_flg);

    int cigar_prod_sem = set_sem(cigar_prod_key, 8, sem_flg);
    int paper_prod_sem = set_sem(paper_prod_key, 8, sem_flg);
    int glu_prod_sem = set_sem(glu_prod_key, 8, sem_flg);

    int cigar_cons_mtx_sem = set_sem(cigar_cons_mtx_key, 1, sem_flg);
    int paper_cons_mtx_sem = set_sem(paper_cons_mtx_key, 1, sem_flg);
    int glu_cons_mtx_sem = set_sem(glu_cons_mtx_key, 1, sem_flg);

    if (type == 1)
    {
        while (1)
        {
            down(paper_cons_sem);
            down(glu_cons_sem);
            down(paper_cons_mtx_sem);
            down(glu_cons_mtx_sem);

            sleep(1);
            printf("smoker %d has got paper%c and glu%c\n",
                    type,
                    paper_buf_ptr[*paper_get_ptr],
                    glu_buf_ptr[*glu_get_ptr]);
            *paper_get_ptr = (*paper_get_ptr + 1) % buff_num;
            *glu_get_ptr = (*glu_get_ptr + 1) % buff_num;

            up(paper_cons_mtx_sem);
            up(glu_cons_mtx_sem);
            up(paper_prod_sem);
            up(glu_prod_sem);
        }
    }

    if (type == 2)
    {
        while (1)
        {
            down(cigar_cons_sem);
            down(glu_cons_sem);
            down(cigar_cons_mtx_sem);
            down(glu_cons_mtx_sem);

            sleep(1);
            printf("smoker %d has got cigar%c and glu%c\n",
                    type,
                    cigar_buf_ptr[*cigar_get_ptr],
                    glu_buf_ptr[*glu_get_ptr]);
            *cigar_get_ptr = (*cigar_get_ptr + 1) % buff_num;
            *glu_get_ptr = (*glu_get_ptr + 1) % buff_num;

            up(cigar_cons_mtx_sem);
            up(glu_cons_mtx_sem);
            up(cigar_prod_sem);
            up(glu_prod_sem);
        }
    }

    if (type == 3)
    {
        while (1)
        {
            down(paper_cons_sem);
            down(cigar_cons_sem);
            down(paper_cons_mtx_sem);
            down(cigar_cons_mtx_sem);

            sleep(1);
            printf("smoker %d has got paper%c and cigar%c\n",
                    type,
                    paper_buf_ptr[*paper_get_ptr],
                    cigar_buf_ptr[*cigar_get_ptr]);
            *paper_get_ptr = (*paper_get_ptr + 1) % buff_num;
            *cigar_get_ptr = (*cigar_get_ptr + 1) % buff_num;

            up(paper_cons_mtx_sem);
            up(cigar_cons_mtx_sem);
            up(paper_prod_sem);
            up(cigar_prod_sem);
        }
    }

    return EXIT_SUCCESS;
}