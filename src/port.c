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
#include <fcntl.h>
#include "configuration.h"
#include "headerport.h"
#define ACCESS 0600
/*----VARIABILI GLOBALI----*/
int sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_semaphore;
struct port *ptr_shm_porto;
struct good *ptr_shm_good;
struct var_conf *ptr_shm_v_conf;
struct sembuf sops;
struct sigaction sa;
struct good *offerta_days[SO_DAYS];
struct good *domanda_days[SO_DAYS];
int *ptr_shm_sem;
int id_porto;
int id_shm_domanda, id_shm_offerta;
int days_real;
int type_offered, type_asked, ton_days;
key_t portMessageQueueKey;
int id_msg;
void cleanup()
{
    printf("ID MSG: %i\n", id_msg);
    if (msgctl(id_msg, IPC_RMID, NULL) == -1)
    {
        perror("PORT: ERROR delete msg");
        exit(1);
    }
    if (id_shm_offerta != -1)
    {
        if (shmctl(id_shm_offerta, IPC_RMID, NULL) == -1)
        {
            perror("PORT: ERROR shmctl in cleanup offerta port");
            exit(1);
        }
        printf("PORT: Memoria condivisa deallocata.\n");
    }
    if (id_shm_domanda != -1)
    {
        if (shmctl(id_shm_domanda, IPC_RMID, NULL) == -1)
        {
            perror("PORT: ERROR shmctl in cleanup  domanda port");
            exit(1);
        }
        printf("PORT: Memoria condivisa deallocata.\n");
    }

    if (shmdt(ptr_shm_good) == -1)
    {
        perror("PORT: ERROR ptr_shm_good in port ");
        exit(1);
    }
    if (shmdt(ptr_shm_v_conf) == -1)
    {
        perror("PORT: ERROR ptr_shm_conf in port");
        exit(1);
    }
    if (shmdt(ptr_shm_porto) == -1)
    {
        perror("PORT: ERROR  ptr_shm_porto in port");
        exit(1);
    }
    if (shmdt(ptr_shm_sem) == -1)
    {
        perror("PORT: ERROR ptr_shm_sem in port");
        exit(1);
    }

    exit(0);
}

void handle_kill_signal(int signum)
{
    printf("PORTO %i: Ricevuto segnale di KILL (%d).\n", id_porto, signum);
    check_good(domanda_days, offerta_days, ptr_shm_v_conf, ptr_shm_porto, ton_days, type_offered, type_asked, id_porto);
    cleanup();
}

/*-------FUNZIONE MAIN-------*/
int main(int argc, char *argv[])
{
    /*----make a connection with master's sh memory---*/
    int i, j;
    double ton_days;
    size_t size1, size2;
    unsigned int random_key;
    sh_mem_id_good = atoi(argv[1]);
    sh_mem_id_conf = atoi(argv[2]);
    sh_mem_id_port = atoi(argv[3]);
    sh_mem_id_semaphore = atoi(argv[4]);
    ptr_shm_good = shmat(sh_mem_id_good, NULL, ACCESS);
    if (ptr_shm_good == NULL)
    {
        perror("Failed to attach ptr_shm_good in port ");
        exit(1);
    }
    ptr_shm_v_conf = shmat(sh_mem_id_conf, NULL, ACCESS);
    if (ptr_shm_v_conf == NULL)
    {
        perror("Failed to attach ptr_shm_v_conf in port ");
        exit(1);
    }
    ptr_shm_porto = shmat(sh_mem_id_port, NULL, ACCESS);
    if (ptr_shm_porto == NULL)
    {
        perror("Failed to attach ptr_shm_porto in port ");
        exit(1);
    }
    ptr_shm_sem = shmat(sh_mem_id_semaphore, NULL, ACCESS);
    if (ptr_shm_sem == NULL)
    {
        perror("Failed to attach ptr_shm_sem in port ");
        exit(1);
    }
    srand(time(NULL));
    days_real = 0;
    port_sorting(ptr_shm_v_conf, ptr_shm_porto);
    for (i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        if (getpid() == ptr_shm_porto[i].pid)
        {
            id_porto = i;
            printf("PORTO %i: ptr_id_porto==[%i]\n", id_porto, ptr_shm_porto[i].id_port);
        }
    }
    /*---------------------------------------------------------------------------------------------*/
    int random_fd = open("/dev/urandom", 0400);
    if (random_fd == -1)
    {
        perror("open /dev/urandom");
        exit(1);
    }

    if (read(random_fd, &random_key, sizeof(random_key)) == -1)
    {
        perror("read /dev/urandom");
        close(random_fd);
        exit(1);
    }

    close(random_fd);

    key_t portMessageQueueKey = random_key;
    // Store the message queue key in the struct port
    ptr_shm_porto[i].message_queue_key = portMessageQueueKey;
    if ((id_msg = msgget(portMessageQueueKey, 0600 | IPC_CREAT)) == -1)
    {
        perror("Failed to create/open message queue for port");
        exit(1);
    }
    /*---------------------------------------------------------------------------------------------*/
    type_offered = (rand() % (ptr_shm_v_conf->so_merci - 1)) + 1;          /*-1 perchè almeno potrò avere la domanda di almeno una merce*/
    type_asked = (rand() % (ptr_shm_v_conf->so_merci - type_offered)) + 1; /*non ho il rischio di avere gli stessi tipi di merce */
    ptr_shm_porto[id_porto].n_type_asked = type_asked;
    ptr_shm_porto[id_porto].n_type_offered = type_offered;
    /*
    creo una memoria condivisa per organizzare domanda e offerta-per ogni porto
    */
    size1 = sizeof(struct good) * type_offered * ptr_shm_v_conf->so_days;
    size2 = sizeof(struct good) * type_asked * ptr_shm_v_conf->so_days;

    id_shm_offerta = shmget(IPC_PRIVATE, size1, ACCESS);
    id_shm_domanda = shmget(IPC_PRIVATE, size2, ACCESS);
    /**salvo il memoria condivisa  il numero dei tipi offerti utilizzati nello scambio di merce con le navi*/
    ptr_shm_porto[id_porto].id_shm_offerta = id_shm_offerta;
    ptr_shm_porto[id_porto].id_shm_domanda = id_shm_domanda;
    /*memoria condivisa per offerta e domanda generate ogni giorno, visibile alle navi*/
    for (i = 0; i < ptr_shm_v_conf->so_days; i++)
    {
        offerta_days[i] = shmat(id_shm_offerta, NULL, 0);
    }
    for (i = 0; i < ptr_shm_v_conf->so_days; i++)
    {
        domanda_days[i] = shmat(id_shm_domanda, NULL, 0);
    }

    /*imposto l'handler*/
    if (signal(SIGINT, handle_kill_signal) == SIG_ERR) /*verificare se me lo esegu anche nel momento in cui io non lo sto chiamando*/
    {
        printf("ricezione segnale nel porto\n");
        exit(1);
    }
    /*-----
    creo la merce e i lotti
    -----*/
    ton_days = (ptr_shm_v_conf->so_fill / ptr_shm_v_conf->so_porti / ptr_shm_v_conf->so_days);
    printf("PORTO %i: tonnellate al giorno----> [%f]\n", id_porto, ton_days);
    create_goods(ptr_shm_v_conf, ptr_shm_good, domanda_days, offerta_days, type_offered, type_asked);
    for (i = 0; i < ptr_shm_v_conf->so_days; i++)
    {
        create_lots(domanda_days, offerta_days, ton_days, type_offered, type_asked, id_porto, i);
    }
    /*--------------DUMP----------------*/
    for (j = 0; j < type_offered; j++)
    {
        printf("DUMP offerta: Porto [%i] -->  merci [%i] --> [%i] lotti \n", id_porto, offerta_days[0][j].id, offerta_days[0][j].lotti);
    }
    for (j = 0; j < type_asked; j++)
    {
        printf("DUMP domanda: Porto [%i] -->  merce [%i] --> [%i] lotti \n", id_porto, domanda_days[0][j].id, domanda_days[0][j].lotti);
    }
    /*
    creo i messaggi da mandare alla nave che accede allo scambio della merce e gli mando l'id della mm condivisa del porto
    */

    /*---------------------------------------------------------------------------*/
    printf("PORTO %i: sto per avvertire il master di essere pronto\n", id_porto);
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = 1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-----PORTO %i: CONFIGURATO, PRONTO ALLA SIMULAZIONE-----\n", id_porto);
    /*---------------------------------------------------------------------------*/
    sops.sem_num = START_SIMULATION;
    sops.sem_op = -1;
    semop(ptr_shm_sem[2], &sops, 1);
    printf("-------PORTO %i: START SIMULAZIONE------\n", id_porto);
    /*----START SIMULAZIONE-----*/
    for (i = 0;; i++)
    {
        sleep(1);
        receive_message(id_msg);
        expired_good(offerta_days, ptr_shm_good, ptr_shm_v_conf, type_offered, id_porto, ptr_shm_v_conf->days_real);
    }
    return 0;
}
