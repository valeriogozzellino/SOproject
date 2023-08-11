#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "configuration.h"

int sh_mem_id_conf;
int sh_mem_id_port;
int sh_mem_id_ship;
int sh_mem_id_semaphore;
int *ptr_shm_sem;
struct port *ptr_shm_port;
struct ship *ptr_shm_ship;
struct var_conf *ptr_shm_v_conf;
struct sembuf sops;

void main(int argc, char *argv[])
{
    sh_mem_id_conf = atoi(argv[1]);
    sh_mem_id_port = atoi(argv[2]);
    sh_mem_id_ship = atoi(argv[3]);
    sh_mem_id_semaphore = atoi(argv[4]);
    ptr_shm_v_conf = shmat(sh_mem_id_conf, NULL, 0600);
    ptr_shm_port = shmat(sh_mem_id_port, NULL, 0600);
    ptr_shm_ship = shmat(sh_mem_id_ship, NULL, 0600);
    ptr_shm_sem = shmat(sh_mem_id_semaphore, NULL, 0600);
    srand(time(NULL));
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("STORM:  configurata, pronta per partire\n");
    /*----attendo il via dal master----*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-------STORM : START SIMULATION-------\n");

    for (int i = 0; i < 50; i++)
    {
        int random_ship = (rand() % (ptr_shm_v_conf->so_navi - i));
        sleep(1);
        if (kill(ptr_shm_ship[random_ship].pid, SIGINT) != -1)
        {
            printf("segnale di terminazione inviato correttamente alla nave %i\n", ptr_shm_ship[random_ship].id_ship);
        }
    }
}
