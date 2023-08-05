/*processo nave*/
/* come eseguire: gcc ship.c , gcc -o ship ship.c*/
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
#include "math.h"
#include "configuration.h"
#include "headership.h"

/*---------VARIABILI GLOBALI-------------*/
struct var_conf *ptr_shm_v_conf;
struct good *ptr_shm_good;
struct good *stiva;
struct port *ptr_shm_port;
struct ship *ptr_shm_ship;
struct sembuf sops;
struct good *offerta_porto;
struct good *domanda_porto;
int *id_porto;
int *ptr_shm_sem;
int sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_ship, sh_mem_id_semaphore;
int id_sem_banchina;
void cleanup()
{

    // Deallocazione della memoria condivisa
    free(stiva);
    free(offerta_porto);
    free(domanda_porto);
    free(id_porto);
    if (shmdt(ptr_shm_good) == -1)
    {
        perror("ptr_shm_good in port ");
        exit(1);
    }
    if (shmdt(ptr_shm_v_conf) == -1)
    {
        perror("ptr_shm_conf in port");
        exit(1);
    }
    if (shmdt(ptr_shm_ship) == -1)
    {
        perror("ptr_shm_ship in port");
        exit(1);
    }
    if (shmdt(ptr_shm_sem) == -1)
    {
        perror("ptr_shm_sem in port");
        exit(1);
    }

    exit(0);
}

// Handler per il segnale di KILL
void handle_kill_signal(int signum)
{
    printf("Ricevuto segnale di KILL (%d).\n", signum);
    cleanup();
}

/*----funzione di movimento della nave verso il porto più vicino----*/

/*--------MAIN --------*/
void main(int argc, char *argv[])
{
    int id_ship, merce_scambiata;
    id_porto = malloc(sizeof(int));
    double tmp_load;
    struct timespec *nano_load;
    srand(time(NULL));
    /*----arguments from master's execve----*/
    sh_mem_id_good = atoi(argv[1]);
    sh_mem_id_conf = atoi(argv[2]);
    sh_mem_id_port = atoi(argv[3]);
    sh_mem_id_ship = atoi(argv[4]);
    sh_mem_id_semaphore = atoi(argv[5]);
    printf("configurazione della nave id_shm:sh_mem_id_good[%i],sh_mem_id_conf[%i],sh_mem_id_port[%i],sh_mem_id_ship[%i],sh_mem_id_semaphore[%i]\n  ", sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_ship, sh_mem_id_semaphore);
    /*-----conversion and attached of shared memory----*/
    ptr_shm_v_conf = shmat(sh_mem_id_conf, NULL, 0400);
    ptr_shm_good = shmat(sh_mem_id_good, NULL, 0600);
    ptr_shm_port = shmat(sh_mem_id_port, NULL, 0400);
    ptr_shm_ship = shmat(sh_mem_id_ship, NULL, 0600);
    ptr_shm_sem = shmat(sh_mem_id_semaphore, NULL, 0600);
    /*-----alloco un array per il trasporto di merci-----*/
    stiva = malloc(sizeof(struct good) * ptr_shm_v_conf->so_merci); // alloco spazio per trasportare  la merce
    sh_memory_v_good(*ptr_shm_v_conf, stiva);
    /**
     * ricerco il pid tra le navi per matchare i dati e avere l'id_ship
     */
    for (int i = 0; i < ptr_shm_v_conf->so_navi; i++)
    {
        if (getpid() == ptr_shm_ship[i].pid)
        {
            ptr_shm_ship[i].id_ship = i;
            id_ship = i;
        }
    }
    printf("ID_SHIP[%d]\n", id_ship);
    /*setto l'handler per la terminazione*/
    if (signal(SIGINT, handle_kill_signal) == SIG_ERR)
    {
        perror("signal");
        exit(1);
    }
    /*-------finito la configurazione------*/
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("nave numero %i configurata, pronta per partire, PID[%i]\n", id_ship, getpid());
    /*----attendo il via dal master----*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-------START SIMULATION FOR THE SHIP [%i]-------\n", id_ship);
    /*eseguita la prima volta per RAGGIUNGERE IL PRIMO PORTO DALLA POSIZIONE CASUALE, SUCCESSIVAMENTE SI SPOSTERÀ DI PORTO IN PORTO*/
    ship_move_first_position(ptr_shm_ship, ptr_shm_port, ptr_shm_v_conf, id_porto, id_ship);
    printf("------> la nave %i si trova al porto : %i \n", id_ship, *id_porto);
    /*--------execution and ship's job------*/
    for (int i = 0;; i++)
    {
        // PRINT DI VERIFICA SE LA NAVE E IL PORTO ABBIAMO LE POSIAZIONI CHE COMBACINO
        printf("SHIP: la nave [%i] --RICHIEDE-- l'accesso  alla banchina del porto[%i]\n", id_ship, *id_porto);
        /*controllo se il porto ha banchine libere altrimenti cambio porto*/
        sops.sem_num = *id_porto;
        sops.sem_op = -1;
        sops.sem_flg = IPC_NOWAIT;
        printf("SHIP: la nave [%i] --HA OTTENUTO-- l'accesso alla banchina del porto [%i]\n", id_ship, *id_porto);
        /**
         *
         */
        if (semop(ptr_shm_sem[0], &sops, 1) != -1)
        { /* se diverso ho avuto accesso alla banchina quindi posso se possibile accedere alla shm*/
            /*INSERIRE LA MSGET PER OTTENERE DAI PORTI L'ID DELLA MEMORIA CONDIVISA*/
            printf("SHIP: la nave (%i) sta  per richiedere l'accesso alla memoria condivisa del porto %i\n", id_ship, *id_porto);
            ptr_shm_ship[id_ship].location = 1; // segnalo di essere in un porto
            sops.sem_num = *id_porto;
            sops.sem_op = -1;
            sops.sem_flg = 0;
            if (semop(ptr_shm_sem[1], &sops, 1) != -1)
            { /*accedo alla memoria condivisa per lo scambio della merce*/
                /*controllo in memroia condivisa se il porto ha richiesta/domanda*/
                printf("SHIP: la nave %i, è attaccata al porto %i, sta per effettuare scambi di merce\n", id_ship, *id_porto);
                // mi sono collegato alla memoria condivisa del porto
                offerta_porto = shmat(ptr_shm_port[*id_porto].id_shm_offerta, NULL, 0600);
                domanda_porto = shmat(ptr_shm_port[*id_porto].id_shm_domanda, NULL, 0600);
                // verifico la domanda, poi prendo la domanda per svuotare la nave
                merce_scambiata = 0;
                if (ptr_shm_ship[id_ship].capacity != ptr_shm_v_conf->so_capacity) // VERIFICO CHE LA MERCER ABBIA ANCORA SPAZIO IN STIVa
                {
                    for (int j = 0; j < ptr_shm_v_conf->so_merci; j++)
                    {
                        printf("-------1------\n");
                        for (int z = 0; z < ptr_shm_port[*id_porto].n_type_asked; z++) // VERIFICARE QUESTI DUE CICLI SONO POCO EFFICIENTI,SAREBBE PIU CORRETTO METTERE UNA BOOLEAN CHE SE TROVA LA MERCE SI FERMA??
                        {
                            printf("-------2------\n");

                            if (stiva[j].id == domanda_porto[z].id)
                            {
                                while ((stiva[j].lotti > 0) && (domanda_porto[z].lotti > 0))
                                {
                                    printf("la nave %i sta soddifando la domanda del porto %i, la domanda  della merce [id= %i]è %i\n", id_ship, *id_porto, stiva[j].id, domanda_porto[z].lotti);
                                    stiva[j].lotti--;
                                    domanda_porto[z].lotti--;
                                    ptr_shm_ship[id_ship].capacity += domanda_porto[z].size; // di quanto decremento la Capacity della size o del LOtto?
                                    printf("sto aumentando la capacita di: %i\n", domanda_porto[z].size);
                                    ptr_shm_port[*id_porto].g_received++;
                                    merce_scambiata = merce_scambiata + stiva[j].size;
                                }
                                printf("la domanda per questa merce ora è: %i\n", domanda_porto[z].lotti);
                            }
                        }
                    }
                    // ERRORE QUI CAPIRE PERCHééééé
                    // tmp_load = merce_scambiata / ptr_shm_v_conf->so_loadspeed;
                    // time_t tmp_seconds = (time_t)tmp_load;
                    // long tmp_nanoseconds = (long)((tmp_load - tmp_seconds) * 1e9);
                    // nano_load->tv_sec = tmp_seconds;
                    // nano_load->tv_nsec = tmp_nanoseconds;
                    // nanosleep(nano_load, NULL);
                }
                merce_scambiata = 0;
                if (ptr_shm_ship[id_ship].capacity != 0) // se la nave ha ancora spazio verifico se ci sono merci che ci stanno
                {

                    printf("-------4------\n");
                    // per ogni merce offerta nel porto oppure fino a quando la stiva ha spazio
                    for (int i = 0; i < ptr_shm_v_conf->so_merci; i++) // verifico per ogni id della merce
                    {
                        printf("-------5------\n");
                        for (int j = 0; j < ptr_shm_port[*id_porto].n_type_offered; j++)
                        {

                            if (offerta_porto[j].id == stiva[i].id) // se sono sulla stessa merce e l'offerta è maggiore di zero
                            {
                                printf("-------6------\n");

                                while (((ptr_shm_ship[id_ship].capacity - offerta_porto[j].size) > 0) && offerta_porto[j].lotti > 0) // entro fino a quando i lotti di quegli id ci stanno nella stiva
                                {
                                    printf("-------7------\n"); // rimane in looop quiiii----------------------------

                                    offerta_porto[j].lotti--;
                                    stiva[i].lotti++;
                                    ptr_shm_ship[id_ship].capacity -= offerta_porto[j].size;
                                    ptr_shm_port[*id_porto].g_send++;
                                    merce_scambiata = merce_scambiata + offerta_porto[j].size;
                                }
                                printf("------------8--------\n");
                            }
                        }
                        printf("--------9-------\n");
                    }
                    printf("---------10--------\n");
                    // ERRORE QUIIIIIIIIIIIIII capire perchè

                    // tmp_load = merce_scambiata / ptr_shm_v_conf->so_loadspeed;
                    // time_t tmp_seconds = (time_t)tmp_load;
                    // long tmp_nanoseconds = fabs((long)((tmp_load - tmp_seconds) * 1e9));
                    // nano_load->tv_sec = tmp_seconds;
                    // nano_load->tv_nsec = tmp_nanoseconds;
                    // nanosleep(nano_load, NULL);
                    printf("---------11---------\n");
                }

                shmdt(offerta_porto);
                shmdt(domanda_porto);
                sops.sem_num = 0;
                sops.sem_op = 1;
                sops.sem_flg = 0;
                semop(ptr_shm_sem[1], &sops, 1); /*aggiungo al semaforo della memoria condivisa*/
                printf("ho terminato di interagire con il porto, ora rilascio la risorsa della memoria condivisa\n");
            }
            sops.sem_num = *id_porto;
            sops.sem_op = 1;
            sops.sem_flg = 0;
            semop(ptr_shm_sem[0], &sops, 1);
            ptr_shm_ship[id_ship].location = 0;
            printf("la nave %i ha lasciato il porto\n", getpid());
        }
        printf("SHIP:  la nave (%i) sta lasciando un porto\n", id_ship);
        printf("TEST sto entrando nella ship move to\n");
        ptr_shm_ship[id_ship].location = 0;
        ship_move_to(ptr_shm_ship, ptr_shm_port, ptr_shm_v_conf, id_porto, id_ship);
    }
}
