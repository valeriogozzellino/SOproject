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
int sh_mem_id_good;
int sh_mem_id_conf;
int sh_mem_id_port;
int sh_mem_id_ship;
int sh_mem_id_semaphore;
int *ptr_shm_sem;
struct port *ptr_shm_port;
struct ship *ptr_shm_ship;
struct var_conf *ptr_shm_v_conf;
struct good *ptr_shm_good;
struct sembuf sops;
void handle_kill_signal(int signum);
void cleanup();

void cleanup()
{
    if (shmdt(ptr_shm_good) == -1)
    {
        perror("ptr_shm_good in DUMP ");
        exit(1);
    }
    if (shmdt(ptr_shm_v_conf) == -1)
    {
        perror("ptr_shm_conf in DUMP");
        exit(1);
    }
    if (shmdt(ptr_shm_port) == -1)
    {
        perror("ptr_shm_porto in DUMP");
        exit(1);
    }
    if (shmdt(ptr_shm_sem) == -1)
    {
        perror("ptr_shm_sem in DUMP");
        exit(1);
    }

    exit(0);
}

void handle_kill_signal(int signum)
{
    switch (signum)
    {

    case SIGINT:
        printf("DUMP : Ricevuto segnale di KILL (%d).\n", signum);
        cleanup();
    }
}

void main(int argc, char *argv[])
{
    int i, j, z;
    sh_mem_id_good = atoi(argv[1]);
    sh_mem_id_conf = atoi(argv[2]);
    sh_mem_id_port = atoi(argv[3]);
    sh_mem_id_ship = atoi(argv[4]);
    sh_mem_id_semaphore = atoi(argv[5]);
    ptr_shm_good = shmat(sh_mem_id_good, NULL, 0600);
    ptr_shm_v_conf = shmat(sh_mem_id_conf, NULL, 0600);
    ptr_shm_port = shmat(sh_mem_id_port, NULL, 0600);
    ptr_shm_ship = shmat(sh_mem_id_ship, NULL, 0600);
    ptr_shm_sem = shmat(sh_mem_id_semaphore, NULL, 0600);
    srand(time(NULL));
    /*-----dichiarazioni di variabili-----*/
    int ship_cariche, ship_vuote, ship_porto, ship_maelstorm, ship_storm, ship_swell;
    struct good *status = malloc(sizeof(struct good) * ptr_shm_v_conf->so_days);
    if (signal(SIGINT, handle_kill_signal) == SIG_ERR)
    {
        printf("ricezione segnale nel DUMP\n");
        exit(1);
    }
    /*-------inizio processo-------*/
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("DUMP:  configurated, ready to go\n");
    /*----attendo il via dal master----*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("----DUMP : START SIMULATION----\n");
    /*---chiamare una funzione dump oer gestire il dump a terminazione ------*/
    for (i = 0; i < SO_DAYS; i++)
    {
        sleep(1);
        ship_cariche = 0, ship_vuote = 0, ship_porto = 0, ship_maelstorm = 0, ship_storm = 0, ship_swell = 0;
        for (j = 0; j < ptr_shm_v_conf->so_navi; j++)
        {
            if (ptr_shm_ship[j].sink_check != 1)
            {
                if (ptr_shm_ship[j].location == 1)
                { /*ship on sea*/
                    if (ptr_shm_ship[j].capacity < ptr_shm_v_conf->so_capacity)
                    {
                        ship_cariche++;
                    }
                    else
                    {
                        ship_vuote++;
                    }
                }
                else
                {
                    ship_porto++;
                }
            }
            else
            {
                if (ptr_shm_ship[j].sink_type.maelstorm > 0)
                {
                    ship_maelstorm++;
                }
                if (ptr_shm_ship[j].sink_type.storm > 0)
                {
                    ship_storm++;
                }
                if (ptr_shm_ship[j].sink_type.swell > 0)
                {
                    ship_swell++;
                }
            }
        }
        printf("DUMP: there are :\n [%d] ship full on sea,\n [%d]ship empty on sea \n [%d] ship in the port \n [%d] ship hit by maelstorm\n [%d] ship  hit by storm\n [%d] ship hit by swell\n", ship_cariche, ship_vuote, ship_porto, ship_maelstorm, ship_storm, ship_swell);
        printf("DUMP: current DAY: %i\n", ptr_shm_v_conf->days_real);
        /**
         *  verifico quante merci sono scadute in mare o in porto
         */
        for (j = 0; j < ptr_shm_v_conf->so_merci; j++)
        {
            printf("DUMP: goods id[%i] , goods life[%i],  lotti scaduti in porto:[%i], lotti scaduti in mare:[%i] \n", ptr_shm_good[j].id, ptr_shm_good[j].life, ptr_shm_good[j].recap.port_expired, ptr_shm_good[j].recap.ship_expired);
        }
    }
}
