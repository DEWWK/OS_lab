#include <unistd.h>
#include "ipc.h"

int main(int argc, char* argv[])
{
    int type = atoi(argv[1]);

    key_t cigar_buf_key = 101;
    key_t paper_buf_key = 201;
    key_t glu_buf_key   = 301;
    buff_num = 8;

    key_t cigar_put_key = 106;
    key_t paper_put_key = 206;
    key_t glu_put_key   = 306;
    cget_num = 1;

    shm_flg = IPC_CREAT | 0644;

    char* cigar_buf_ptr = (char*)set_shm(cigar_buf_key, buff_num, shm_flg);
    char* paper_buf_ptr = (char*)set_shm(paper_buf_key, buff_num, shm_flg);
    char* glu_buf_ptr   = (char*)set_shm(glu_buf_key,   buff_num, shm_flg);

    int* cigar_put_ptr = (int*)set_shm(cigar_put_key, cget_num, shm_flg);
    int* paper_put_ptr = (int*)set_shm(paper_put_key, cget_num, shm_flg);
    int* glu_put_ptr   = (int*)set_shm(glu_put_key,   cget_num, shm_flg);

    key_t cigar_cons_key = 103;
    key_t cigar_prod_key = 104;
    key_t cigar_prod_mtx_key = 107;

    key_t paper_cons_key = 203;
    key_t paper_prod_key = 204;
    key_t paper_prod_mtx_key = 207;

    key_t glu_cons_key = 303;
    key_t glu_prod_key = 304;
    key_t glu_prod_mtx_key = 307;

    sem_flg = IPC_CREAT | 0644;

    int cigar_cons_sem = set_sem(cigar_cons_key, 0, sem_flg);
    int paper_cons_sem = set_sem(paper_cons_key, 0, sem_flg);
    int glu_cons_sem = set_sem(glu_cons_key, 0, sem_flg);

    int cigar_prod_sem = set_sem(cigar_prod_key, 8, sem_flg);
    int paper_prod_sem = set_sem(paper_prod_key, 8, sem_flg);
    int glu_prod_sem = set_sem(glu_prod_key, 8, sem_flg);

    int cigar_prod_mtx_sem = set_sem(cigar_prod_mtx_key, 1, sem_flg);
    int paper_prod_mtx_sem = set_sem(paper_prod_mtx_key, 1, sem_flg);
    int glu_prod_mtx_sem = set_sem(glu_prod_mtx_key, 1, sem_flg);

    int i = 1;
    while (i)
    {
        if (i == 1)
        {
            down(cigar_prod_sem);
            down(paper_prod_sem);
            down(cigar_prod_mtx_sem);
            down(paper_prod_mtx_sem);

            cigar_buf_ptr[*cigar_put_ptr] = 'A' + *cigar_put_ptr;
            paper_buf_ptr[*paper_put_ptr] = 'A' + *paper_put_ptr;
            sleep(1);
            printf("provider %d put cigar%c and paper%c\n",
                    type,
                    cigar_buf_ptr[*cigar_put_ptr],
                    paper_buf_ptr[*paper_put_ptr]);
            *paper_put_ptr = (*paper_put_ptr + 1) % buff_num;
            *cigar_put_ptr = (*cigar_put_ptr + 1) % buff_num;

            up(cigar_cons_sem);
            up(paper_cons_sem);
            up(cigar_prod_mtx_sem);
            up(paper_prod_mtx_sem);
        }

        if (i == 2)
        {
            down(glu_prod_sem);
            down(paper_prod_sem);
            down(glu_prod_mtx_sem);
            down(paper_prod_mtx_sem);

            glu_buf_ptr[*glu_put_ptr] = 'A' + *glu_put_ptr;
            paper_buf_ptr[*paper_put_ptr] = 'A' + *paper_put_ptr;
            sleep(1);
            printf("provider %d put glu%c and paper%c\n",
                    type,
                    glu_buf_ptr[*glu_put_ptr],
                    paper_buf_ptr[*paper_put_ptr]);
            *paper_put_ptr = (*paper_put_ptr + 1) % buff_num;
            *glu_put_ptr = (*glu_put_ptr + 1) % buff_num;

            up(glu_cons_sem);
            up(paper_cons_sem);
            up(glu_prod_mtx_sem);
            up(paper_prod_mtx_sem);
        }

        if (i == 3)
        {
            down(cigar_prod_sem);
            down(glu_prod_sem);
            down(cigar_prod_mtx_sem);
            down(glu_prod_mtx_sem);

            cigar_buf_ptr[*cigar_put_ptr] = 'A' + *cigar_put_ptr;
            glu_buf_ptr[*glu_put_ptr] = 'A' + *glu_put_ptr;
            sleep(1);
            printf("provider %d put cigar%c and glu%c\n",
                    type,
                    cigar_buf_ptr[*cigar_put_ptr],
                    glu_buf_ptr[*glu_put_ptr]);
            *glu_put_ptr = (*glu_put_ptr + 1) % buff_num;
            *cigar_put_ptr = (*cigar_put_ptr + 1) % buff_num;

            up(cigar_cons_sem);
            up(glu_cons_sem);
            up(cigar_prod_mtx_sem);
            up(glu_prod_mtx_sem);
        }

        i ++ ;
        if (i > 3) i = 1;
    }
    
    return EXIT_SUCCESS;
}