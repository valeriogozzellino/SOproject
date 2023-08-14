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
#include "headerport.h"
#define ACCESS 0600
/*----VARIABILI GLOBALI----*/
int sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_semaphore;
struct port *ptr_shm_porto; 
struct good *ptr_shm_good;  // ptr to shared mem for info good
struct good **domanda_days; // per ogni giorno vengono create le merci, array di array in cui il primo rafppresenta il giorno e il secondo le merci
struct good **offerta_days;
struct var_conf *ptr_shm_v_conf;
struct sembuf sops;
struct sigaction sa;
int *ptr_shm_sem;
int id_porto;
int id_shm_domanda, id_shm_offerta;

// void handle_signal_termination(int signal);
// Funzione per liberare la memoria condivisa
void cleanup()
{

    // Deallocazione della memoria condivisa
    if (id_shm_offerta != -1)
    {
        if (shmctl(id_shm_offerta, IPC_RMID, NULL) == -1)
        {
            perror("shmctl in cleanup offerta port");
            exit(1);
        }
        printf("Memoria condivisa deallocata.\n");
    }
    if (id_shm_domanda != -1)
    {
        if (shmctl(id_shm_domanda, IPC_RMID, NULL) == -1)
        {
            perror("shmctl in cleanup  domanda port");
            exit(1);
        }
        printf("Memoria condivisa deallocata.\n");
    }

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
    if (shmdt(ptr_shm_porto) == -1)
    {
        perror("ptr_shm_porto in port");
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
    printf("PORTO %i: Ricevuto segnale di KILL (%d).\n", id_porto, signum);
    cleanup();
}

/*-------FUNZIONE MAIN-------*/
void main(int argc, char *argv[])
{
    /*----make a connection with master's sh memory---*/
    sh_mem_id_good = atoi(argv[1]);
    sh_mem_id_conf = atoi(argv[2]);
    sh_mem_id_port = atoi(argv[3]);
    sh_mem_id_semaphore = atoi(argv[4]);
    // printf("SONO NEL PORTO : shm_id_good[%i], sh_mem_id_conf[%i], sh_mem_id_port[%i], sh_mem_id_semaphore[%i]\n", sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_semaphore);
    ptr_shm_good = shmat(sh_mem_id_good, NULL, ACCESS);
    ptr_shm_v_conf = shmat(sh_mem_id_conf, NULL, ACCESS);
    ptr_shm_porto = shmat(sh_mem_id_port, NULL, ACCESS);
    ptr_shm_sem = shmat(sh_mem_id_semaphore, NULL, ACCESS);
    srand(time(NULL));
    double ton_days;

    /*mi riconduco all'id del mio porto per matchare i dati*/
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        if (getpid() == ptr_shm_porto[i].pid)
        {
            id_porto = i;
            printf("PORTO %i: ptr_id_porto==[%i]\n", id_porto, ptr_shm_porto[i].id_port);
        }
    }
    printf("ID_PORTO [%i]\n", id_porto);
    int type_offered = (rand() % (ptr_shm_v_conf->so_merci - 1)) + 1;          /*-1 perchè almeno potrò avere la domanda di almeno una merce*/
    int type_asked = (rand() % (ptr_shm_v_conf->so_merci - type_offered)) + 1; /*non ho il rischio di avere gli stessi tipi di merce */
    ptr_shm_porto[id_porto].n_type_asked = type_asked;
    ptr_shm_porto[id_porto].n_type_offered = type_offered;
    /*
    creo una memoria condivisa per organizzare domanda e offerta-per ogni porto
    */
    id_shm_offerta = shmget(IPC_PRIVATE, (sizeof(struct good)) * ptr_shm_v_conf->so_days, ACCESS); // creo un array di offerte per ogni giorno
    id_shm_domanda = shmget(IPC_PRIVATE, (sizeof(struct good)) * ptr_shm_v_conf->so_days, ACCESS);
    /**salvo il memoria condivisa  il numero dei tipi offerti utilizzati nello scambio di merce con le navi*/
    ptr_shm_porto[id_porto].id_shm_offerta = id_shm_offerta;
    ptr_shm_porto[id_porto].id_shm_domanda = id_shm_domanda;
    /*memoria condivisa per offerta e domanda generate ogni giorno, visibile alle navi*/
    offerta_days = shmat(id_shm_offerta, NULL, ACCESS);
    domanda_days = shmat(id_shm_domanda, NULL, ACCESS);
    for (int i = 0; i < ptr_shm_v_conf->so_days; i++)
    {
        offerta_days[i] = malloc(sizeof(struct good) * type_offered);
        domanda_days[i] = malloc(sizeof(struct good) * type_asked);
    }
    /*imposto l'handler*/
    if (signal(SIGINT, handle_kill_signal) == SIG_ERR) /*verificare se me lo esegu anche nel momento in cui io non lo sto chiamando*/
    {
        printf("ricezione segnale nel porto\n");
        perror("signal\n");
        exit(1);
    }
    /*-----
    creo la merce e i lotti
    -----*/
    ton_days = (ptr_shm_v_conf->so_fill / ptr_shm_v_conf->so_porti / ptr_shm_v_conf->so_days);
    printf("PORTO %i: tonnellate al giorno----> [%f]\n", id_porto, ton_days);
    create_goods(ptr_shm_v_conf, ptr_shm_good, domanda_days, offerta_days, id_shm_domanda, id_shm_offerta, type_offered, type_asked, id_porto);
    /*secondo me non funziona create lotti!!!*/
    create_lots(domanda_days, offerta_days, ton_days, type_offered, type_asked, id_porto, ptr_shm_v_conf->days_real);
    /*
    creo i messaggi da mandare alla nave che accede allo scambio della merce e gli mando l'id della mm condivisa del porto
    */

    /*---------------------------------------------------------------------------*/
    printf("PORTO %i: sto per avvertire il master di essere pronto\n", id_porto);
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1); // attendo master che mi dia il via quando tutti i px hanno aggiunto 1 al ready to go
    printf("-----PORTO %i: CONFIGURATO, PRONTO ALLA SIMULAZIONE-----\n", id_porto);
    /*------------*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-------PORTO %i: START SIMULAZIONE------\n", id_porto);
    /*----START SIMULAZIONE-----*/
    int i = 0;
    while (i <= ptr_shm_v_conf->so_days)
    {
        sleep(1);
        create_lots(domanda_days, offerta_days, ton_days, type_offered, type_asked, id_porto, ptr_shm_v_conf->days_real); /*creazione giornaliera di lotti*/
        i++;
    }
    // impostare un handler con una maschera che se ricevo segnale di terminazone allora maschero il segnale elimino tutto e poi termino
    // sigaction(SIGUSR1, &sa, NULL);
}
