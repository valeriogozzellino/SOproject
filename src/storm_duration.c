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
        perror("STORM: ERROR ptr_shm_conf ");
        exit(1);
    }
    if (shmdt(ptr_shm_port) == -1)
    {
        perror("STORM: ERROR ptr_shm_porto ");
        exit(1);
    }
      if (shmdt(ptr_shm_ship) == -1)
    {
        perror("STORM: ERROR ptr_shm_ship ");
        exit(1);
    }
    if (shmdt(ptr_shm_sem) == -1)
    {
        perror("STORM: ERROR ptr_shm_sem");
        exit(1);
    }

    exit(0);
}

void handle_kill_signal(int signum)
{
    printf("STORM: Ricevuto segnale di KILL .\n");
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
    int random_ship, i;
    if (signal(SIGINT, handle_kill_signal) == SIG_ERR)
    {
        printf("ricezione segnale nel STORM\n");
        exit(1);
    }
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

    for (i = 0;; i++)
    {
        random_ship = rand() % (ptr_shm_v_conf->so_navi);
        //printf("STORM: nave da comlpire : %i\n", random_ship);
        sleep(1);
        if (kill(ptr_shm_ship[random_ship].pid, SIGUSR1) != -1)
        {
            printf("SEGNALE STORM inviato correttamente alla nave %i\n", ptr_shm_ship[random_ship].id_ship);
        }
    }
}
