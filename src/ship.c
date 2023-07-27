/*processo nave*/
/* come eseguire: gcc ship.c , gcc -o ship ship.c*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
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
#include "configuration.h"

/*---------PROTOTIPI DI FUNZIONI----------*/
void ship_move_first_position(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship);
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship);
// void handle_signal_termination(int signal);
/*---------VARIABILI GLOBALI-------------*/
struct var_conf *ptr_shm_v_conf;
struct good *ptr_shm_good;
struct good *stiva;
struct port *ptr_shm_port;
struct ship *ptr_shm_ship;
struct sembuf sops;
struct good *offerta_porto;
struct good *domanda_porto;
int *ptr_shm_sem;
int sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_ship, sh_mem_id_semaphore;
int id_sem_banchina;
void cleanup()
{

    // Deallocazione della memoria condivisa
    free(stiva);
    free(offerta_porto);
    free(domanda_porto);

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
/*DA CONTROLLAREEEEEEEE*/
void ship_move_first_position(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship)
{
    /*----utilities parameter----*/
    double tmp;
    struct timespec *nanotime = malloc(sizeof(struct timespec));
    /*array distanze lo utilizzo per selezionare il porto più vicino*/
    double *array_distance = malloc(sizeof(double) * ptr_shm_v_conf->so_porti);
    int indice_first_port = 0;
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        /*inserisco il valore assoluto*/
        array_distance[i] = sqrt(pow(ptr_shm_port[i].pos_porto.x - ptr_shm_ship[id_ship].pos_ship.x, 2) + pow(ptr_shm_port[i].pos_porto.y - ptr_shm_ship[id_ship].pos_ship.y, 2));
        // printf("distanza porti dalla nave: %f\n", array_distance[i]);
    }
    /*seleziono la distanza minore*/
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {                                                              // seleziono la distanza minore e l'indice del porto per la sua posizione
        if (array_distance[indice_first_port] > array_distance[i]) // controllare l'array delle distanze e in modo da ordinarlo e salvare gli indici nell'arrai degli indici
        {
            indice_first_port = i;
        }
    }
    printf("array distanze creato: porto piu vicino:[%f],primo porto [%f]\n", array_distance[indice_first_port], array_distance[0]);
    id_porto = indice_first_port;
    tmp = ((array_distance[indice_first_port]) / ((double)ptr_shm_v_conf->so_speed));
    printf("la nave farà un viaggio in mare che durerà %f secondi\n", tmp);
    nanotime->tv_nsec = (time_t)tmp;                /*casto a intero così prendo solo la prima parte decimale*/
    nanotime->tv_nsec = (long int)(tmp - (int)tmp); /*posso farlo per avere solo la parte decimale? */
    nanosleep(nanotime, NULL);
    /*----ora devo modificare la x e la y della nave----*/
    ptr_shm_ship[id_ship].pos_ship.x = ptr_shm_port[indice_first_port].pos_porto.x;
    ptr_shm_ship[id_ship].pos_ship.y = ptr_shm_port[indice_first_port].pos_porto.y;
    printf("nave:[%i]---->  x_aggiornato:%f, y_aggiornato:%f", id_ship, ptr_shm_ship[id_ship].pos_ship.x, ptr_shm_ship[id_ship].pos_ship.y);
    free(array_distance);
    free(nanotime);
}
/*------funzione che mi permette di spostarmi nella mappa da porto a porto------*/
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship)
{
    /*----utilities parameter----*/
    double tmp;
    struct timespec *nanotime = malloc(sizeof(struct timespec));
    if (id_porto >= ptr_shm_v_conf->so_porti - 1) // verificare la correttezza
    {
        tmp = ((sqrt(pow(ptr_shm_port[0].pos_porto.x - ptr_shm_ship[id_ship].pos_ship.x, 2) + pow(ptr_shm_port[0].pos_porto.y - ptr_shm_ship[id_ship].pos_ship.y, 2))) / ((double)ptr_shm_v_conf->so_speed));
        printf("la nave farà un viaggio in mare che durerà %f secondi\n", tmp);
        nanotime->tv_sec = (time_t)tmp;                 /*casto a intero così prendo solo la prima parte decimale*/
        nanotime->tv_nsec = (long int)(tmp - (int)tmp); /*posso farlo per avere solo la parte decimale? */
        nanosleep(nanotime, NULL);
        id_porto++;
    }
    else
    {
        tmp = ((DISTANZA_P_N_) / ((double)ptr_shm_v_conf->so_speed));
        printf("la nave farà un viaggio in mare che durerà %f secondi\n", tmp);
        nanotime->tv_nsec = (time_t)tmp;                /*casto a intero così prendo solo la prima parte decimale*/
        nanotime->tv_nsec = (long int)(tmp - (int)tmp); /*posso farlo per avere solo la parte decimale? */
        nanosleep(nanotime, NULL);
        id_porto = 0;
    }
    /*----ora devo modificare la x e la y della nave----*/
    ptr_shm_ship[id_ship].pos_ship.x = ptr_shm_port[id_porto].pos_porto.x;
    ptr_shm_ship[id_ship].pos_ship.y = ptr_shm_port[id_porto].pos_porto.y;
    printf("posizione della nave[%i] aggiornata:(%f,%f), si trova al porto[%i]", id_ship, ptr_shm_ship[id_ship].pos_ship.x, ptr_shm_ship[id_ship].pos_ship.y, id_porto);
    free(nanotime);
}
/*--------MAIN --------*/
void main(int argc, char *argv[])
{
    int id_ship, id_porto, merce_scambiata;
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
    /*--------execution and ship's job------*/
    for (int i = 0;; i++)
    {
        printf("sono al porto con posizione: pos_porto[%f, %f], pos_ship[%f,%f]\n", ptr_shm_port[id_porto].pos_porto.x, ptr_shm_port[id_porto].pos_porto.y, ptr_shm_ship[id_ship].pos_ship.x, ptr_shm_ship[id_ship].pos_ship.y);
        /*controllo se il porto ha banchine libere altrimenti cambio porto*/
        sops.sem_num = id_porto;
        sops.sem_op = -1;
        sops.sem_flg = IPC_NOWAIT;
        if (semop(ptr_shm_sem[0], &sops, 1) != -1)
        { /* se diverso ho avuto accesso alla banchina quindi posso se possibile accedere alla shm*/
            /*INSERIRE LA MSGET PER OTTENERE DAI PORTI L'ID DELLA MEMORIA CONDIVISA*/
            printf("sono nella banchina sto per richiedere l'accesso alla memoria condivisa\n");
            ptr_shm_ship[id_ship].location = 1; // segnalo di essere in un porto
            sops.sem_num = id_porto;
            sops.sem_op = -1;
            sops.sem_flg = 0;
            if (semop(ptr_shm_sem[1], &sops, 1) != -1)
            { /*accedo alla memoria condivisa per lo scambio della merce*/
                /*controllo in memroia condivisa se il porto ha richiesta/domanda*/
                printf("la nave è attaccata al porto, sono nell'if");
                // mi sono collegato alla memoria condivisa del porto
                offerta_porto = shmat(ptr_shm_port[id_porto].id_shm_offerta, NULL, 0600);
                domanda_porto = shmat(ptr_shm_port[id_porto].id_shm_domanda, NULL, 0600);
                // verifico la domanda, poi prendo la domanda per svuotare la nave
                if (ptr_shm_ship[id_ship].capacity != 0)
                {
                    merce_scambiata = 0;
                    for (int j = 0; j < ptr_shm_v_conf->so_merci; i++)
                    {
                        for (int z = 0; z < ptr_shm_port[id_porto].n_type_asked; i++)
                        {
                            if (stiva[j].id == domanda_porto[z].id)
                            {
                                printf("la nave %i sta soddifando la domanda del porto %i, la domanda era %i\n", id_ship, id_porto, domanda_porto[z].lotti);
                                while ((stiva[z].lotti > 0) && (domanda_porto[z].lotti > 0))
                                {
                                    stiva[z].lotti--;
                                    domanda_porto[z].lotti--;
                                    ptr_shm_ship[id_ship].capacity--;
                                    printf("sto decrementando la capacità\n");
                                    ptr_shm_port[id_porto]
                                        .g_received++;
                                    merce_scambiata = merce_scambiata + stiva[z].size;
                                }
                                printf("la domanda per questa merce ora è: %i\n", domanda_porto[z].lotti);
                            }
                        }
                    }
                    tmp_load = merce_scambiata / ptr_shm_v_conf->so_loadspeed;
                    nano_load->tv_sec = (int)tmp_load;
                    nano_load->tv_nsec = (long int)(tmp_load - (int)tmp_load);
                    nanosleep(nano_load, NULL);
                }
                merce_scambiata = 0;
                for (int j = 0; (j < ptr_shm_port[id_porto].n_type_offered) || (ptr_shm_ship[id_ship].capacity == ptr_shm_v_conf->so_capacity); i++)
                {
                    for (int i = 0; i < ptr_shm_v_conf->so_merci; i++)
                    {
                        if (offerta_porto[j].id == stiva[i].id)
                        {
                            while ((ptr_shm_ship[id_ship].capacity != ptr_shm_v_conf->so_capacity) && (offerta_porto[j].id > 0))
                            {
                                offerta_porto[j].lotti--;
                                stiva[i].lotti++;
                                ptr_shm_ship[id_ship].capacity++;
                                ptr_shm_port[id_porto].g_send++;
                                merce_scambiata = merce_scambiata + stiva[i].size;
                            }
                        }
                    }
                }
                tmp_load = merce_scambiata / ptr_shm_v_conf->so_loadspeed;
                nano_load->tv_sec = (int)tmp_load;
                nano_load->tv_nsec = (long int)(tmp_load - (int)tmp_load);
                nanosleep(nano_load, NULL);

                shmdt(offerta_porto);
                shmdt(domanda_porto);
                sops.sem_num = 0;
                sops.sem_op = 1;
                sops.sem_flg = 0;
                semop(ptr_shm_sem[1], &sops, 1); /*aggiungo al semaforo della memoria condivisa*/
                printf("ho terminato di interagire con il porto, ora rilascio la risorsa della memoria condivisa\n");
            }
            sops.sem_num = id_porto;
            sops.sem_op = 1;
            sops.sem_flg = 0;
            semop(ptr_shm_sem[0], &sops, 1);
            ptr_shm_ship[id_ship].location = 0;
            printf("la nave %i ha lasciato il porto\n", getpid());
        }
        ship_move_to(ptr_shm_ship, ptr_shm_port, ptr_shm_v_conf, id_porto, id_ship);
    }
}
