/*processo nave*/
/* come eseguire: gcc ship.c , gcc -o ship ship.c*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "math.h"
#include "configuration.h"
#include "headership.h"

/*---------VARIABILI GLOBALI-------------*/
struct var_conf *ptr_shm_v_conf;
struct good *ptr_shm_good;
struct good **stiva;
struct port *ptr_shm_port;
struct ship *ptr_shm_ship;
struct sembuf sops;

int *id_porto;
int id_ship;
int *ptr_shm_sem;
int sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_ship, sh_mem_id_semaphore;
int id_sem_banchina;
int portMessageQueue;
void cleanup()
{

    free(stiva);
    free(id_porto);
    if (shmdt(ptr_shm_good) == -1)
    {
        perror("SHIP: ERROR ptr_shm_good in ship \n");
        exit(1);
    }
    if (shmdt(ptr_shm_v_conf) == -1)
    {
        perror("SHIP: ERROR ptr_shm_conf in ship\n");
        exit(1);
    }
    if (shmdt(ptr_shm_ship) == -1)
    {
        perror("SHIP: ERROR ptr_shm_ship in ship\n");
        exit(1);
    }
    if (shmdt(ptr_shm_sem) == -1)
    {
        perror("ptr_shm_sem in ship\n");
        exit(1);
    }
    // if (msgctl(portMessageQueue, IPC_RMID, NULL) == -1)
    // {
    //     perror("Failed to deallocate message queue\n");
    //     exit(1);
    // }

    exit(0);
}

void handle_kill_signal(int signum)
{
    double tmp_storm, tmp_swell;
    struct timespec nano_load;
    switch (signum)
    {
    case SIGUSR1:
        tmp_storm = ptr_shm_v_conf->so_storm_duration;
        nano_load.tv_sec = (int)tmp_storm;                             /*take seconds*/
        nano_load.tv_nsec = (tmp_storm - (int)tmp_storm) * 1000000000; /*take nanoseconds*/
        nanosleep(&nano_load, NULL);
        ptr_shm_ship[id_ship].sink_type.storm++;
        printf("SHIP: Ricevuto seganle di STORM alla nave \n");
        break;
    case SIGUSR2:
        tmp_swell = ptr_shm_v_conf->so_swell_duration;
        nano_load.tv_sec = (int)tmp_swell;                             /*take seconds*/
        nano_load.tv_nsec = (tmp_swell - (int)tmp_swell) * 1000000000; /*take nanoseconds*/
        nanosleep(&nano_load, NULL);
        ptr_shm_ship[id_ship].sink_type.swell++;
        printf("SHIP: Ricevuto seganle di SWELL alla nave \n");
        break;
    case SIGINT:
        printf("SHIP: Ricevuto segnale di KILL.\n");
        ptr_shm_ship[id_ship].sink_check = 1;
        ptr_shm_ship[id_ship].sink_type.maelstorm++;
        /*rilascio le risorse dei porti dei semafori*/
        if (ptr_shm_ship[id_ship].location == 1)
        {

            sops.sem_num = *id_porto;
            sops.sem_op = 1;
            sops.sem_flg = 0;
            semop(ptr_shm_sem[0], &sops, 1);
            if (ptr_shm_ship[id_ship].in_exchange == 1)
            {
                sops.sem_num = *id_porto;
                sops.sem_op = 1;
                sops.sem_flg = 0;
                semop(ptr_shm_sem[1], &sops, 1);
            }
        }
        ptr_shm_ship[id_ship].location = 0;
        cleanup();
        break;
    }
}

/*----funzione di movimento della nave verso il porto più vicino----*/

/*--------MAIN --------*/
int main(int argc, char *argv[])
{
    int i, j, z;
    double merce_scambiata;
    double tmp_load;
    struct timespec nano_load;
    srand(time(NULL));
    id_porto = malloc(sizeof(int));
    /*----arguments from master's execve----*/
    sh_mem_id_good = atoi(argv[1]);
    sh_mem_id_conf = atoi(argv[2]);
    sh_mem_id_port = atoi(argv[3]);
    sh_mem_id_ship = atoi(argv[4]);
    sh_mem_id_semaphore = atoi(argv[5]);
    /*-----conversion and attached of shared memory----*/
    ptr_shm_v_conf = shmat(sh_mem_id_conf, NULL, 0400);
    if (ptr_shm_v_conf == NULL)
    {
        perror("Error in attached ptr_shm_v_conf\n");
        exit(1);
    }
    ptr_shm_good = shmat(sh_mem_id_good, NULL, 0600);
    if (ptr_shm_good == NULL)
    {
        perror("Error in attached ptr_shm_good\n");
        exit(1);
    }
    ptr_shm_port = shmat(sh_mem_id_port, NULL, 0400);
    if (ptr_shm_port == NULL)
    {
        perror("Error in attached ptr_shm_port\n");
        exit(1);
    }
    ptr_shm_ship = shmat(sh_mem_id_ship, NULL, 0600);
    if (ptr_shm_ship == NULL)
    {
        perror("Error in attached ptr_shm_ship\n");
        exit(1);
    }
    ptr_shm_sem = shmat(sh_mem_id_semaphore, NULL, 0600);
    if (ptr_shm_sem == NULL)
    {
        perror("Error in attached ptr_shm_sem\n");
        exit(1);
    }
    /*-----alloco un array per il trasporto di merci-----*/
    stiva = malloc(sizeof(struct good) * ptr_shm_v_conf->so_days);
    struct good *domanda_porto[SO_DAYS];
    struct good *offerta_porto[SO_DAYS];
    for (i = 0; i < ptr_shm_v_conf->so_days; i++)
    {
        stiva[i] = malloc(sizeof(struct good) * ptr_shm_v_conf->so_merci);
    }
    /**
     * setto le caratteristiche delle merci nella stiva
     */
    set_good_ship(ptr_shm_good, stiva, *ptr_shm_v_conf);
    /**
     * ricerco il pid tra le navi per matchare i dati e avere l'id_ship
     */
    for (i = 0; i < ptr_shm_v_conf->so_navi; i++)
    {
        if (getpid() == ptr_shm_ship[i].pid)
        {
            ptr_shm_ship[i].id_ship = i;
            id_ship = i;
        }
    }
    /**
     * setto gli handler dei segnali
     */
    if (signal(SIGINT, handle_kill_signal) == SIG_ERR)
    {
        perror("ERRORE storm signal");
        exit(1);
    }
    if (signal(SIGUSR1, handle_kill_signal) == SIG_ERR)
    {
        perror("ERRORE swell signal");
        exit(1);
    }
    if (signal(SIGUSR2, handle_kill_signal) == SIG_ERR)
    {
        perror("ERRORE terminazione signal");
        exit(1);
    }
    /*-------finito la configurazione------*/
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("SHIP %i:  configurata, pronta per partire, PID[%i]\n", id_ship, getpid());
    /*----attendo il via dal master----*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-------SHIP %i: START SIMULATION-------\n", id_ship);
    ptr_shm_ship[id_ship].sink_check = 0;
    /*eseguita la prima volta per RAGGIUNGERE IL PRIMO PORTO DALLA POSIZIONE CASUALE, SUCCESSIVAMENTE SI SPOSTERÀ DI PORTO IN PORTO*/
    ship_move_first_position(ptr_shm_ship, ptr_shm_port, ptr_shm_v_conf, id_porto, id_ship);
    /*--------execution and ship's job------*/
    for (i = 0;; i++)
    {
        // printf("SHIP %i:  --RICHIEDE-- l'accesso  alla banchina del porto[%i]\n", id_ship, *id_porto);
        sops.sem_num = *id_porto;
        sops.sem_op = -1;
        sops.sem_flg = IPC_NOWAIT;
        if (semop(ptr_shm_sem[0], &sops, 1) != -1)
        {
            ptr_shm_port[*id_porto].banchine_occupate++;
            sendAttackMessage(ptr_shm_port[*id_porto].message_queue_key, "Attack on the quay!");
            ship_expired_good(ptr_shm_ship, ptr_shm_v_conf, ptr_shm_good, id_ship, stiva);
            // printf("SHIP %i:  sta  per richiedere l'accesso alla memoria condivisa del porto %i\n", id_ship, *id_porto);
            ptr_shm_ship[id_ship].location = 0;
            sops.sem_num = *id_porto;
            sops.sem_op = -1;
            sops.sem_flg = 0;
            if (semop(ptr_shm_sem[1], &sops, 1) != -1)
            {
                // printf("SHIP %i: è attaccata al porto %i, sta per effettuare scambi di merce\n", id_ship, *id_porto);
                /*-------salvo il pid della nave per inviare segnali in caso di swell--------*/
                ptr_shm_ship[id_ship].in_exchange = 1;
                ptr_shm_port[*id_porto].pid_ship = ptr_shm_ship[id_ship].pid;
                for (i = 0; i < ptr_shm_v_conf->so_days; i++)
                {
                    offerta_porto[i] = shmat(ptr_shm_port[*id_porto].id_shm_offerta, NULL, 0);
                    domanda_porto[i] = shmat(ptr_shm_port[*id_porto].id_shm_domanda, NULL, 0);
                }

                merce_scambiata = 0;
                if (ptr_shm_ship[id_ship].capacity != ptr_shm_v_conf->so_capacity)
                {

                    for (i = 0; i <= ptr_shm_v_conf->days_real; i++)
                    {

                        for (j = 0; j < ptr_shm_v_conf->so_merci; j++)
                        {
                            for (z = 0; z < ptr_shm_port[*id_porto].n_type_asked; z++)
                            {
                                if (stiva[i][j].id == domanda_porto[ptr_shm_v_conf->days_real][z].id)
                                {
                                    while ((stiva[i][j].lotti > 0) && (domanda_porto[ptr_shm_v_conf->days_real][z].lotti > 0))
                                    {
                                        printf("SHIP %i:  sta soddifando la domanda del porto %i, la domanda  della merce [id= %i]è %i\n", id_ship, *id_porto, stiva[i][j].id, domanda_porto[ptr_shm_v_conf->days_real][z].lotti);
                                        stiva[i][j].lotti--;
                                        domanda_porto[ptr_shm_v_conf->days_real][z].lotti--;
                                        domanda_porto[ptr_shm_v_conf->days_real][z].recap.consegnata = 1;
                                        ptr_shm_ship[id_ship].capacity += domanda_porto[ptr_shm_v_conf->days_real][z].size;
                                        ptr_shm_port[*id_porto].g_received += offerta_porto[ptr_shm_v_conf->days_real][z].size;
                                        merce_scambiata = merce_scambiata + domanda_porto[ptr_shm_v_conf->days_real][z].size;
                                    }
                                }
                            }
                        }
                    }
                    if (merce_scambiata != 0)
                    {
                        tmp_load = merce_scambiata / ptr_shm_v_conf->so_loadspeed;
                        printf("SHIP %i: tempo di scarica : %f \n", id_ship, tmp_load);
                        nano_load.tv_sec = (int)tmp_load;                            /*take seconds*/
                        nano_load.tv_nsec = (tmp_load - (int)tmp_load) * 1000000000; /*take nanoseconds*/
                        nanosleep(&nano_load, NULL);
                    }
                }
                merce_scambiata = 0;
                if (ptr_shm_ship[id_ship].capacity != 0)
                {
                    for (i = 0; i <= ptr_shm_v_conf->days_real; i++)
                    {
                        for (j = 0; j < ptr_shm_v_conf->so_merci; j++)
                        {
                            for (z = 0; z < ptr_shm_port[*id_porto].n_type_offered; z++)
                            {

                                if (offerta_porto[ptr_shm_v_conf->days_real][z].id == stiva[i][j].id)
                                {
                                    while (((ptr_shm_ship[id_ship].capacity - offerta_porto[ptr_shm_v_conf->days_real][z].size) > 0) && offerta_porto[ptr_shm_v_conf->days_real][z].lotti > 0)
                                    {
                                        // printf("SHIP %i: sta soddifando l'offerta del porto %i, l'offerta  della merce [id= %i]è %i\n", id_ship, *id_porto, stiva[i][j].id, offerta_porto[ptr_shm_v_conf->days_real][z].lotti);
                                        offerta_porto[ptr_shm_v_conf->days_real][z].lotti--;
                                        offerta_porto[ptr_shm_v_conf->days_real][z].recap.ritirata = 1;
                                        stiva[i][j].lotti++;
                                        ptr_shm_ship[id_ship].capacity -= offerta_porto[ptr_shm_v_conf->days_real][z].size;
                                        ptr_shm_port[*id_porto].g_send += offerta_porto[ptr_shm_v_conf->days_real][z].size;
                                        merce_scambiata = merce_scambiata + offerta_porto[ptr_shm_v_conf->days_real][z].size;
                                        // printf("SHIP %i: MERCE SCAMBIATA : %f \n", id_ship, merce_scambiata);
                                    }
                                }
                            }
                        }
                    }
                    if (merce_scambiata != 0)
                    {
                        tmp_load = merce_scambiata / ptr_shm_v_conf->so_loadspeed;
                        // printf("SHIP %i: tempo di carica : %f \n", id_ship, tmp_load);
                        nano_load.tv_sec = (int)tmp_load;                            /*take seconds*/
                        nano_load.tv_nsec = (tmp_load - (int)tmp_load) * 1000000000; /*take nanoseconds*/
                        nanosleep(&nano_load, NULL);
                    }
                }

                shmdt(offerta_porto);
                shmdt(domanda_porto);
                sops.sem_num = *id_porto;
                sops.sem_op = 1;
                sops.sem_flg = 0;
                semop(ptr_shm_sem[1], &sops, 1); /*aggiungo al semaforo della memoria condivisa*/
                ptr_shm_ship[id_ship].in_exchange = 0;
                /*--------la nave non sta più scambiando merce--------*/
                ptr_shm_port[*id_porto].pid_ship = 0;
                // printf("Ship: la nave %i ho terminato di interagire con il porto, ora rilascio la risorsa della memoria condivisa\n", id_ship);
            }
            sops.sem_num = *id_porto;
            sops.sem_op = 1;
            sops.sem_flg = 0;
            semop(ptr_shm_sem[0], &sops, 1);
            ptr_shm_port[*id_porto].banchine_occupate--;
            ptr_shm_ship[id_ship].location = 1;
        }
        // printf("SHIP:  la nave (%i) sta lasciando un porto %i\n", id_ship, *id_porto);
        ship_move_to(ptr_shm_ship, ptr_shm_port, ptr_shm_v_conf, id_porto, id_ship);
    }
    return 0;
}
