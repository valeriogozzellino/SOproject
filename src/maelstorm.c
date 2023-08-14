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
void handle_kill_signal(int signum);
void cleanup();
/**
 *
 */
void cleanup()
{
    if (shmdt(ptr_shm_v_conf) == -1)
    {
        perror("ptr_shm_conf in MAELSTORM");
        exit(1);
    }
    if (shmdt(ptr_shm_port) == -1)
    {
        perror("ptr_shm_porto in MAELSTORM");
        exit(1);
    }
    if (shmdt(ptr_shm_sem) == -1)
    {
        perror("ptr_shm_sem in MAELSTORM");
        exit(1);
    }

    exit(0);
}

void handle_kill_signal(int signum)
{
    printf("MAELSTORM : Ricevuto segnale di KILL.\n");
    cleanup();
}

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
    struct timespec nano_load;
    int random_ship, i;
    double tmp_sleep = ptr_shm_v_conf->so_maelstorm;
    /**
     *
     */
    if (signal(SIGINT, handle_kill_signal) == SIG_ERR)
    {
        printf("ricezione segnale nel maelstorm\n");
        exit(1);
    }
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("MAELSTORM:  configurata\n");
    /*----attendo il via dal master----*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-------MAELSTORM : START SIMULATION-------\n");

    for (i = 0; i < ptr_shm_v_conf->so_navi; i++)
    {
        random_ship = (rand() % (ptr_shm_v_conf->so_navi - i));
        nano_load.tv_sec = (int)tmp_sleep;                             /*take seconds*/
        nano_load.tv_nsec = (tmp_sleep - (int)tmp_sleep) * 1000000000; /*take nanoseconds*/
        printf("MAELSTORM: kill della nave tra: %f", tmp_sleep);
        nanosleep(&nano_load, NULL);
        if (kill(ptr_shm_ship[random_ship].pid, SIGINT) != -1)
        {
            printf("SEGNALE MAELSTORM inviato correttamente alla nave %i\n", ptr_shm_ship[random_ship].id_ship);
        }
    }
}
