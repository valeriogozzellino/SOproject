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

/*----VARIABILI GLOBALI----*/
int sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_semaphore;
struct port *ptr_shm_porto; // ptr to shared memroy info porto
struct good *ptr_shm_good;  // ptr to shared mem for info good
struct good *domanda;
struct good *offerta;
struct var_conf *ptr_shm_v_conf;
struct sembuf sops;
struct sigaction sa;
int *ptr_shm_sem;
int id_porto;
int id_shm_domanda, id_shm_offerta;

/*----PROTOTIPI DI FUNZIONI-----*/
void create_goods(struct var_conf *ptr_shm_v_conf, struct good *ptr_shm_good, struct good *domanda, struct good *offerta, int id_shm_domanda, int id_shm_offertaint, int type_offered, int type_asked, int id_porto);
void create_lots(struct good *domanda, struct good *offerta, int ton_days, int type_offered, int type_asked, int id_porto);
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
    printf("Ricevuto segnale di KILL (%d).\n", signum);
    cleanup();
}

void create_goods(struct var_conf *ptr_shm_var_conf, struct good *ptr_shm_good, struct good *domanda, struct good *offerta, int id_shm_domanda, int id_shm_offerta, int type_offered, int type_asked, int id_porto)
{
    printf("Ho %i tipi di merce offerta al porto %i\n", type_offered, id_porto);
    printf("Ho %i tipi di merce domandata al porto %i\n", type_asked, id_porto);

    // Creazione delle quantità dei tipi di merce offerti
    for (int i = 0; i < type_offered; i++)
    {
        offerta[i].id = rand() % ptr_shm_var_conf->so_merci;
        offerta[i].size = ptr_shm_good[offerta[i].id].size;
        offerta[i].life = ptr_shm_good[offerta[i].id].life;
        printf("Tipi offerti---> merce id: (%i), size(%i), life(%i)\n", offerta[i].id, offerta[i].size, offerta[i].life);
    }

    // Creazione delle quantità dei tipi di merce domandati
    for (int i = 0; i < type_asked; i++)
    {
        domanda[i].id = rand() % ptr_shm_var_conf->so_merci;
        domanda[i].size = ptr_shm_good[domanda[i].id].size;
        printf("Tipi domandati----> merce id: (%i), size(%i)\n", domanda[i].id, domanda[i].size);
    }
}

void create_lots(struct good *domanda, struct good *offerta, int ton_days, int type_offered, int type_asked, int id_porto)
{
    double ton_disp = ton_days / 2;
    double ton_disp2 = ton_days / 2;
    double ton_disp3;

    // Creazione dei lotti per le merci offerte
    int creazione = 1;
    while (creazione != 0)
    {
        creazione = 0;
        for (int j = 0; j < type_offered; j++)
        {
            if (offerta[j].size < ton_disp)
            {
                creazione = 1;
                offerta[j].lotti++;
                ton_disp -= offerta[j].size;
            }
        }
    }
    // Creazione dei lotti per le merci domandate
    creazione = 1;
    while (creazione != 0)
    {
        creazione = 0;
        for (int j = 0; j < type_asked; j++)
        {
            if (domanda[j].size < ton_disp2)
            {
                creazione = 1;
                domanda[j].lotti++;
                ton_disp2 -= domanda[j].size;
            }
        }
    }

    // Controllo per l'uso degli avanzi di tonnellaggio
    ton_disp3 = ton_disp2 + ton_disp;
    for (int j = 0; j < type_offered; j++)
    {
        if (offerta[j].size < ton_disp3)
        {
            ton_disp3 -= offerta[j].size;
            offerta[j].lotti++;
        }
    }
    for (int j = 0; j < type_asked; j++)
    {
        if (domanda[j].size < ton_disp2 && domanda[j].size < ton_disp3)
        {
            ton_disp3 -= domanda[j].size;
            domanda[j].lotti++;
        }
    }

    // Stampa dei risultati
    // for (int j = 0; j < type_asked; j++)
    // {
    //     printf("Porto [%i], merce-type [%i], lotti domandati (%i)\n", id_porto, domanda[j].id, domanda[j].lotti);
    // }
    // for (int j = 0; j < type_offered; j++)
    // {
    //     printf("Porto [%i], merce-type [%i], lotti in offerta creati (%i)\n", id_porto, offerta[j].id, offerta[j].lotti);
    // }
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
    ptr_shm_good = shmat(sh_mem_id_good, NULL, 0600);
    ptr_shm_v_conf = shmat(sh_mem_id_conf, NULL, 0600);
    ptr_shm_porto = shmat(sh_mem_id_port, NULL, 0600);
    ptr_shm_sem = shmat(sh_mem_id_semaphore, NULL, 0600);
    srand(time(NULL));
    double ton_days;

    /*mi riconduco all'id del mio porto per matchare i dati*/
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        if (getpid() == ptr_shm_porto[i].pid)
        {
            id_porto = i;
            printf("id_porto==[%i], ptr_id_porto==[%i]\n", id_porto, ptr_shm_porto[i].id_port);
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
    id_shm_offerta = shmget(IPC_PRIVATE, (sizeof(struct good)) * type_offered, 0600);
    id_shm_domanda = shmget(IPC_PRIVATE, (sizeof(struct good)) * type_asked, 0600);
    /**salvo il memoria condivisa  il numero dei tipi offerti utilizzati nello scambio di merce con le navi*/
    ptr_shm_porto[id_porto].id_shm_offerta = id_shm_offerta;
    ptr_shm_porto[id_porto].id_shm_domanda = id_shm_domanda;
    /*memoria condivisa per offerta e domanda, visibile alle navi*/
    offerta = shmat(id_shm_offerta, NULL, 0600);
    domanda = shmat(id_shm_domanda, NULL, 0600);
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
    printf("tonnellate al giorno PORTO[%i]----> [%f]\n", id_porto, ton_days);
    create_goods(ptr_shm_v_conf, ptr_shm_good, domanda, offerta, id_shm_domanda, id_shm_offerta, type_offered, type_asked, id_porto);
    /*secondo me non funziona create lotti!!!*/
    create_lots(domanda, offerta, ton_days, type_offered, type_asked, id_porto);
    /*
    creo i messaggi da mandare alla nave che accede allo scambio della merce e gli mando l'id della mm condivisa del porto
    */

    /*---------------------------------------------------------------------------*/
    printf("sono un porto, sto per avvertire il master di essere pronto\n ");
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1); // attendo master che mi dia il via quando tutti i px hanno aggiunto 1 al ready to go
    printf("-----PORTI CONFIGURATI, PRONTI ALLA SIMULAZIONE-----\n");
    /*------------*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-------START SIMULAZIONE PORTI------\n");
    /*----START SIMULAZIONE-----*/
    int i = 0;
    while (i <= ptr_shm_v_conf->so_days)
    {
        sleep(1);
        create_lots(domanda, offerta, ton_days, type_offered, type_asked, id_porto); /*creazione giornaliera di lotti*/
        i++;
    }
    // impostare un handler con una maschera che se ricevo segnale di terminazone allora maschero il segnale elimino tutto e poi termino
    // sigaction(SIGUSR1, &sa, NULL);
}

// /*problemi: - non cancello la mem condivisa le malloc e la msg ,lo devo fare quando ricevo il segnale di terminazione dal master
// perciò devo mascherare il segnale
// - ogni volta che passa un giorno posso inviare un segnale dal master e richiedere di inviare le informazioni utili per il recap
// girnaliero
// - il file porto non viene eseguito, ho un errore nella messaget all'inizio del file
// */
