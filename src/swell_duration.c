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
        perror("SWELL: ERROR ptr_shm_conf in swell_duration");
        exit(1);
    }
    if (shmdt(ptr_shm_port) == -1)
    {
        perror("SWELL: ERROR ptr_shm_porto in swell_duration");
        exit(1);
    }
    if (shmdt(ptr_shm_ship) == -1)
    {
        perror("SWELL: ERROR ptr_shm_ship in swell_duration");
        exit(1);
    }
    if (shmdt(ptr_shm_sem) == -1)
    {
        perror("SWELL: ERROR ptr_shm_sem in swell_duration");
        exit(1);
    }

    exit(0);
}

void handle_kill_signal(int signum)
{
    printf("SWELL: Ricevuto segnale di KILL.\n");
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
    if (signal(SIGINT, handle_kill_signal) == SIG_ERR)
    {
        printf("SWELL: ricezione segnale nel swell_duration\n");
        exit(1);
    }
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("SWELL:  configurata, pronta per partire\n");
    /*----attendo il via dal master----*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-------SWELL_DURATION: START SIMULATION-------\n");

    for (int i = 0; i < ptr_shm_v_conf->so_days; i++)
    {
        /*capire come fermare lo scambio delle merci mandando il segnale alla nave */
        int random_port = (rand() % ptr_shm_v_conf->so_porti);
        sleep(1);
        if (ptr_shm_port[random_port].pid_ship != 0)
        {
            if (kill(ptr_shm_port[random_port].pid_ship, SIGUSR2) != -1)
            {
                printf("SEGNALE SWELL INVIATO correttamente alla nave\n");
            }
        }
        else /*altrimenti significa che non ci sono navi attaccate e semplicemente tolgo la risorsa della memoria condivisa*/
        {
            printf("SWELL RICHIEDE LA RISORSA DELLA MEMORIA CONDIVISA\n");
            sops.sem_num = random_port;
            sops.sem_op = -1;
            sops.sem_flg = 0;
            if (semop(ptr_shm_sem[1], &sops, 1) != -1)
            {
                double tmp_sleep = ptr_shm_v_conf->so_maelstorm;
                nano_load.tv_sec = (int)tmp_sleep;                             /*take seconds*/
                nano_load.tv_nsec = (tmp_sleep - (int)tmp_sleep) * 1000000000; /*take nanoseconds*/
                nanosleep(&nano_load, NULL);
                /*------rilascio la risorsa-------*/
                sops.sem_num = random_port;
                sops.sem_op = 1;
                sops.sem_flg = 0;
                semop(ptr_shm_sem[1], &sops, 1);
            }
        }
    }
}
