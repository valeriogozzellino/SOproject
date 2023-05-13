/*come eseguire : gcc port.c , gcc -o port port.c*/
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
void create_good(struct var_conf *ptr_shm_v_conf, struct good *ptr_shm_good, struct good *domanda, struct good *offerta, int id_shm_domanda, int id_shm_offertaint, int type_offered, int type_asked);
void create_lotti(struct good *domanda, struct good *offerta, int ton_days, int type_offered, int type_asked, int id_porto);
void handle_signal_termination(int signal);
/*-------FUNZIONE MAIN-------*/
void main(int argc, char *argv[])
{
    /*----make a connection with master's sh memory---*/
    sh_mem_id_good = atoi(argv[1]);
    sh_mem_id_conf = atoi(argv[2]);
    sh_mem_id_port = atoi(argv[3]);
    sh_mem_id_semaphore = atoi(argv[4]);
    printf("SONO NEL PORTO:shm_id_good[%i],sh_mem_id_conf[%i],sh_mem_id_port[%i],sh_mem_id_semaphore[%i]\n", sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_semaphore);
    ptr_shm_good = shmat(sh_mem_id_good, NULL, 0600);   // ONLY read
    ptr_shm_v_conf = shmat(sh_mem_id_conf, NULL, 0600); // ONLY read
    ptr_shm_porto = shmat(sh_mem_id_port, NULL, 0600);
    ptr_shm_sem = shmat(sh_mem_id_semaphore, NULL, 0600);

    /*l'ID del porto non lo inserisco in ordine di creazione del porto ma in base alla sua posizione all'interno della mappa
    INSERIRE IN UN HEADER FILE*/
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        if (getpid() == ptr_shm_porto[i].pid)
        {
            id_porto = ptr_shm_porto[i].id_port;
        }
    }
    printf("ID_PORTO[%i]\n", id_porto);
    int type_offered = (rand() % (ptr_shm_v_conf->so_merci - 1)) + 1;          /*-1 perchè almeno potrò avere la domanda di almeno una merce*/
    int type_asked = (rand() % (ptr_shm_v_conf->so_merci - type_offered)) + 1; /*non ho il rischio di avere gli stessi tipi di merce */
    double ton_days;
    srand(time(NULL));
    /*
    creo una memoria condivisa per organizzare domanda e offerta-
    */
    id_shm_offerta = shmget(IPC_PRIVATE, (sizeof(struct good)) * type_offered, 0600);
    id_shm_domanda = shmget(IPC_PRIVATE, (sizeof(struct good)) * type_asked, 0600);
    ptr_shm_porto[id_porto].id_shm_offerta = id_shm_offerta;
    ptr_shm_porto[id_porto].id_shm_domanda = id_shm_domanda;
    printf("shm_offerta=[%i], shm_domanda=[%i]\n ", id_shm_offerta, id_shm_domanda);
    offerta = shmat(id_shm_offerta, NULL, 0600);
    domanda = shmat(id_shm_domanda, NULL, 0600);
    /*imposto l'handler*/
    sa.sa_handler = handle_signal_termination;
    /*-----
    creo i lotti
    -----*/
    ton_days = (ptr_shm_v_conf->so_fill / ptr_shm_v_conf->so_porti / ptr_shm_v_conf->so_days);
    create_good(ptr_shm_v_conf, ptr_shm_good, domanda, offerta, id_shm_domanda, id_shm_offerta, type_offered, type_asked);
    /*secondo me non funziona create lotti!!!*/
    create_lotti(domanda, offerta, ton_days, type_offered, type_asked, id_porto);
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
    printf("START SIMULAZIONE PORTI\n");
    /*----START SIMULAZIONE-----*/
    while (1)
    {

        sleep(1);
        create_lotti(domanda, offerta, ton_days, type_offered, type_asked, id_porto); /*creazione giornaliera di lotti*/

        // rimane in loop fino a quando il gestore non decide di terminarmi
    }
    // impostare un handler con una maschera che se ricevo segnale di terminazone allora maschero il segnale elimino tutto e poi termino
    sigaction(SIGUSR1, &sa, NULL);
}

/*-----CREAZIONE DELLE MERCI --------*/
void create_good(struct var_conf *ptr_shm_var_conf, struct good *ptr_shm_good, struct good *domanda, struct good *offerta, int id_shm_domanda, int id_shm_offerta, int type_offered, int type_asked)
{
    /*scelgo il numero dei tipi di merce che il porto creerà o offrirà, IMP: off e domanda devono avere tipi di merce differente
        il numero e il tipo di merce rimane invariato lungo tutta la simulazione, cambiano solo le quantità generate*/

    printf("ho %i tipi di merce offerta al porto %i\n", type_offered, id_porto);
    printf("ho %i tipi di merce domandata al porto %i\n", type_asked, id_porto);

    /*creo le quantità dei tipi.di.merce*/

    for (int i = 0; i < type_offered; i++) /*inizializzo le merci disponibili*/
    {

        offerta[i].type_offered = type_offered;
        offerta[i].id = (rand() % ptr_shm_v_conf->so_merci); /*sceglie un id, ce ne possono essere o tutti diversi oppure si possono anche ripetere*/
        offerta[i].size = ptr_shm_good[offerta[i].id].size;  /* assegno il peso */
        offerta[i].life = ptr_shm_good[offerta[i].id].life;
        printf("tipi offerti dal porto[%i]: (%i), merce id: (%i), size(%i), life(%i)\n", id_porto, offerta[i].type_offered, offerta[i].id, offerta[i].size, offerta[i].life);
    }
    for (int i = 0; i < type_asked; i++)
    {
        domanda[i].type_asked = type_asked;
        domanda[i].id = (rand() % ptr_shm_v_conf->so_merci) + 1;
        for (int j = 0; j < type_offered; j++)
        { // verifico che l'id non sia già nelle offerte
            if (domanda[i].id == offerta[j].id)
            {
                domanda[i].id = (rand() % ptr_shm_v_conf->so_merci) + 1;
            }
        }
        domanda[i].size = ptr_shm_good[domanda[i].id].size; /* assegno il peso */
        printf("tipi domandati dal porto[%i]: (%i), merce id: (%i), size(%i)\n", id_porto, domanda[i].type_asked, domanda[i].id, domanda[i].size);
    }
}
/*----CREAZIONE DEI LOTTI-------- */
/*cercando di utilizzare il più possibile lo spazio di SO_FILL------*/
void create_lotti(struct good *domanda, struct good *offerta, int ton_days, int type_offered, int type_asked, int id_porto)
{

    int end = 1, end_2;
    double ton_disp = ton_days / 2; // il valore restituito può essere un double??
    double ton_disp2 = ton_days / 2;
    double ton_disp3; // terzo controllo utilizzato per la domma degli avanzi distribuiti.
    for (int j = 0; end == 1; j++)
    {
        if (j == type_offered) /*riparto dall'inizio*/
        {
            j = 0;
        }
        else
        {
            if (offerta[j].size < ton_disp)
            { /*se la size del lotto è minore di sofill allora creo un lotto, altrimenti verifico che ce ne sia almeno 1*/
                offerta[j].lotti++;
                ton_disp -= offerta[j].size;
            }
            else
            {
                end_2 = 0;
                for (int i = 0; (i < type_offered) && (end_2 == 0); i++)
                {
                    if (offerta[i].size < ton_disp)
                    {
                        end_2 = 1;
                    }
                }
                end = end_2;
            }
        }
    }
    end = 1;
    for (int j = 0; end == 1; j++)
    {
        if (j == type_asked)
        {
            j = 0;
        }
        else
        {
            if (domanda[j].size < ton_disp2)
            { /*se la size del lotto è minore di sofill allora creo un lotto, altrimenti verifico che ce ne sia almeno 1*/
                domanda[j].lotti++;
                ton_disp2 -= domanda[j].size;
            }
            else
            {
                end_2 = 0;
                for (int j = 0; (j < type_asked) && (end_2 == 0); j++)
                {
                    if (domanda[j].size < ton_disp2)
                    {
                        end_2 = 1;
                    }
                }
                end = end_2;
            }
        }
    }
    /*faccio un altro controllo con le quantità che posso aver avanzato nelle due distribuzioni iniziali*/
    ton_disp3 = ton_disp2 + ton_disp;
    for (int j = 0; j < type_offered; j++)
    {
        if (offerta[j].size < ton_disp3)
        {
            ton_disp3 -= offerta[j].size;
            offerta[j].lotti++;
        }
    }
    for (int j = 0; (j < type_asked); j++)
    {
        if (domanda[j].size < ton_disp2)
        {
            if (domanda[j].size < ton_disp3)
            {
                ton_disp3 -= domanda[j].size;
                domanda[j].lotti++;
            }
        }
    }
    for (int j = 0; j < type_asked; j++)
    {

        printf(" porto:[%i], merce[%i], lotti domandati (%i)\n", id_porto, j, domanda[j].lotti);
    }
    for (int j = 0; j < type_offered; j++)
    {

        printf("porto[%i], merce[%i], lotti in offerta creati(%i)\n", id_porto, j, offerta[j].lotti);
    }
}

void handle_signal_termination(int signal)
{
    switch (signal)
    {
    case SIGUSR1:
        shmctl(id_shm_domanda, IPC_RMID, NULL);
        shmctl(id_shm_offerta, IPC_RMID, NULL);
        exit(EXIT_SUCCESS);
        break;
    }
}
/*problemi: - non cancello la mem condivisa le malloc e la msg ,lo devo fare quando ricevo il segnale di terminazione dal master
perciò devo mascherare il segnale
- ogni volta che passa un giorno posso inviare un segnale dal master e richiedere di inviare le informazioni utili per il recap
girnaliero
- il file porto non viene eseguito, ho un errore nella messaget all'inizio del file
*/