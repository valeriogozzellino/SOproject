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
    struct good *domanda_porto[ptr_shm_v_conf->so_days];
    struct good *offerta_porto[ptr_shm_v_conf->so_days];
    int ship_cariche = 0, ship_vuote = 0, ship_porto = 0, ship_affondate = 0;
    struct good *status = malloc(sizeof(struct good) * ptr_shm_v_conf->so_days);
    if (signal(SIGINT, handle_kill_signal) == SIG_ERR)
    {
        printf("ricezione segnale nel DUMP\n");
        exit(1);
    }
    /*--------mi attacco alla memoria condivisa dei porti----- */
    for (j = 0; j < ptr_shm_v_conf->so_porti; j++)
    {

        for (i = 0; i < ptr_shm_v_conf->so_days; i++)
        {
            offerta_porto[i] = shmat(ptr_shm_port[j].id_shm_offerta, NULL, 0);
            domanda_porto[i] = shmat(ptr_shm_port[j].id_shm_domanda, NULL, 0);
        }
    }
    /*-------inizio processo-------*/
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("DUMP:  configurato, pronto per partire\n");
    /*----attendo il via dal master----*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-------DUMP : START SIMULATION-------\n");
    /*---chiamare una funzione dump oer gestire il dump a terminazione ------*/
    for (i = 0; i < ptr_shm_v_conf->so_days; i++)
    {
        sleep(1);
        ship_cariche = 0, ship_vuote = 0, ship_porto = 0, ship_affondate = 0;
        for (i = 0; i < ptr_shm_v_conf->so_navi; i++)
        {
            if (ptr_shm_ship[i].affondata != 1)
            {

                if (ptr_shm_ship[i].location == 1)
                { /*navi nel mare*/
                    if (ptr_shm_ship[i].capacity < ptr_shm_v_conf->so_capacity)
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
                ship_affondate++;
            }
        }
        printf("DUMP: ci sono : [%d] navi in mare cariche,\n [%d]navi in mare vuote,\n [%d] navi in porto\n [%d] navi affondate\n", ship_cariche, ship_vuote, ship_porto, ship_affondate);
        printf("DUMP: numero di giorni: %i\n", ptr_shm_v_conf->days_real);
        /**
         *  verifico quante merci sono scadute in mare o in porto
         */
        for (i = 0; i < ptr_shm_v_conf->so_merci; i++)
        {
            printf("DUMP: merce id[%i] , lotti scaduti in mare:[%i], lotti scaduti in porto:[%i] \n", ptr_shm_good[i].id, ptr_shm_good[i].recap.port_expired, ptr_shm_good[i].recap.ship_expired);
        }
        /*
         * verifico quanta merce è presente in un porto
         */
        // for (i = 0; i < ptr_shm_v_conf->so_porti; i++)
        // {
        //     for (j = 0; j < ptr_shm_v_conf->so_merci; j++)
        //     {
        //         for (z = 0; z < ptr_shm_port[i].n_type_offered; z++)
        //         {
        //         }
        //     }
        //     printf("DUMP: il porto %i ha RICEVUTO[%i lotti] e SPEDITO[%i lotti] e ha [%i]Banchine occupate \n", ptr_shm_port[i].id_port, ptr_shm_port[i].g_received, ptr_shm_port[i].g_send, ptr_shm_port[i].banchine_occupate);
        // }
    }
}
