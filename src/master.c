/*MASTER*/
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
#include "configuration.h"
#define NUM_PROCESSI (env_var.so_navi + env_var.so_porti)

/*--------variabili che devo utilizzare---------*/
struct var_conf env_var;
struct good merce;
struct port porto;

/*----puntatori alla memoria condivisa------*/
struct var_conf *ptr_shm_v_conf;
struct good *ptr_shm_good;
struct port *ptr_shm_port;
struct ship *ptr_shm_ship;
struct sembuf sops;
int *ptr_shm_semaphore;
int sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_ship, sh_mem_id_semaphore; // id shared memory
int sem_id_banchine, sem_id_shm, sem_id;                                                 // semafori
int status;

/*------------prototipi di funzioni-----------*/
void create_port(struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf);
void create_ship(struct ship *ptr_shm_ship, struct var_conf *ptr_shm_v_conf);
/*---inizializza i semafori----*/
// void handle_signal(int signal);
void signalHandler(int signum);
/*----------MAIN-----------*/
int main()
{
    /*------ora configuro le variabili-------*/
    signal(SIGINT, signalHandler);
    int ship_cariche, ship_vuote, ship_porto, days_real;
    srand(time(NULL));
    // env_var = malloc(sizeof(struct var_conf));
    /*-----funzioni in file configuration.h----*/

    find_val(&env_var);

    /*-----alloco una memoria condivisa---- */
    if ((sh_mem_id_conf = shmget(IPC_PRIVATE, sizeof(struct var_conf), 0600)) < 0)
    {
        TEST_ERROR;
    }
    printf("id mem cond %i\n", sh_mem_id_conf);
    if ((sh_mem_id_good = shmget(IPC_PRIVATE, (sizeof(struct good) * env_var.so_merci), 0600)) < 0)
    {
        TEST_ERROR;
    }
    printf("id mem good %i\n", sh_mem_id_good);
    if ((sh_mem_id_port = shmget(IPC_PRIVATE, (sizeof(struct port) * env_var.so_porti), 0600)) < 0)
    {
        TEST_ERROR;
    }
    printf("id mem port %i\n", sh_mem_id_port);
    if ((sh_mem_id_ship = shmget(IPC_PRIVATE, (sizeof(struct ship) * env_var.so_navi), 0600)) < 0)
    {
        TEST_ERROR;
    }
    printf("id mem ship %i\n", sh_mem_id_ship);
    if ((sh_mem_id_semaphore = shmget(IPC_PRIVATE, (sizeof(int) * 3), 0600)) < 0)
    {
        TEST_ERROR;
    }
    printf("id mem semaphore %i\n", sh_mem_id_semaphore);

    /*------pointer to shared memory-----*/
    ptr_shm_v_conf = shmat(sh_mem_id_conf, NULL, 0600);
    ptr_shm_good = shmat(sh_mem_id_good, NULL, 0600);
    ptr_shm_port = shmat(sh_mem_id_port, NULL, 0600);
    ptr_shm_ship = shmat(sh_mem_id_ship, NULL, 0600);
    ptr_shm_semaphore = shmat(sh_mem_id_semaphore, NULL, 0600);
    /*------creation of set's semaphore----*/
    if ((sem_id_banchine = semget(IPC_PRIVATE, env_var.so_porti, 0600)) == -1)
    {
        TEST_ERROR;
    }
    if ((sem_id_shm = semget(IPC_PRIVATE, env_var.so_porti, 0600)) == -1)
    {
        TEST_ERROR;
    }
    if ((sem_id = semget(IPC_PRIVATE, 2, 0600)) == -1)
    {
        TEST_ERROR;
    }
    /*-----inizialization configuration variables in sh_memory. (configuration.h)------*/
    sh_memory_v_conf(env_var, ptr_shm_v_conf);
    /*----inizialization of good  in shared memory. (configuration.h)----*/
    sh_memory_v_good(env_var, ptr_shm_good);
    /*-----inizialization parameter for port in configuration-----*/
    sh_memory_v_porti(env_var, ptr_shm_port);
    /*------inizialization parameter for ship in configuration-----*/
    sh_memory_v_ship(env_var, ptr_shm_ship);
    /*------inizializzo le risorse per il semafori------*/
    load_val_semaphor(sem_id_banchine, sem_id_shm, sem_id, ptr_shm_semaphore, ptr_shm_v_conf);

    printf("HO INIZIALIZZATO I VALORI IN MEMORIA CONDIVISA , es: \n semaforo 1:%i, size merce casuale: %i, so fill porto:%i \n", ptr_shm_semaphore[0], ptr_shm_good[4].size, ptr_shm_port[1].fill);
    /*-----creazione dei processi porto-----*/
    create_port(ptr_shm_port, ptr_shm_v_conf);
    /*----ordinamento dei porti, richiamo port_sorting in configuration------*/
    port_sorting(ptr_shm_v_conf, ptr_shm_port);
    printf("sono il master sono uscito dalla port sorting\n");
    /*-----creazione delle navi-----*/
    create_ship(ptr_shm_ship, ptr_shm_v_conf);

    printf("sono il master sono uscito dalla create ship\n");

    /*----ALL PROCESS CREATED-----*/
    printf("numero di processi totali è:");
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = -NUM_PROCESSI;
    semop(sem_id, &sops, 1);
    /*----START SIMULATION-----*/
    sops.sem_flg = 0;
    sops.sem_num = START_SIMULATION;
    sops.sem_op = NUM_PROCESSI;
    semop(sem_id, &sops, 1);
    printf("------------SONO IL MASTER DO IL VIA-----------\n");
    /*----START TEMPO DI SIMULAZIONE, LE NAVI GURANO FINO A RICEZIONE DEL SEGNALE -----*/
    alarm(env_var.so_days);
    while (wait(&status) != -1) // quando terminano tutti i processi
    {
        sleep(1); /*attende un giorno = 1sec*/
        printf("È PASSATO UN GIORNO\n");
        days_real++;
        ship_cariche = ship_vuote = ship_porto = 0;
        for (int i = 0; i < env_var.so_navi; i++)
        {
            if (ptr_shm_ship[i].location == 0)
            { // navi nel mare
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
        printf("ci sono : [%d] navi in mare cariche,\n [%d]navi in mare vuote,\n [%d] navi in porto\n", ship_cariche, ship_vuote, ship_porto);
        printf("numero di giorni: %i\n", days_real);
    }

    printf("sto per cancellare la mem condivisa\n");
    /*--------TERMINAZIONE DELLA SIMULAZIONE-----------------*/
    /*------elimino la memoria condivisa-----*/
    if (shmctl(sh_mem_id_conf, IPC_RMID, NULL) == -1)
    {
        TEST_ERROR;
    }
    if (shmctl(sh_mem_id_good, IPC_RMID, NULL) == -1)
    {
        TEST_ERROR;
    }
    if (shmctl(sh_mem_id_port, IPC_RMID, NULL) == -1)
    {
        TEST_ERROR;
    }
    if (shmctl(sh_mem_id_ship, IPC_RMID, NULL) == -1)
    {
        TEST_ERROR;
    }
    if (shmctl(sh_mem_id_semaphore, IPC_RMID, NULL) == -1)
    {
        TEST_ERROR;
    }
    semctl(sem_id_banchine, 0, IPC_RMID);
    semctl(sem_id_shm, 0, IPC_RMID);
    semctl(sem_id, 0, IPC_RMID);
    // free(env_var);
    printf("ho cancellato tutto \n");
    return 0;
}

void create_port(struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf)
{

    char *id_mem_good = malloc(sizeof(char));
    char *id_mem_conf = malloc(sizeof(char));
    char *id_mem_port = malloc(sizeof(char));
    char *id_mem_semop = malloc(sizeof(char));

    char *args_porto[] = {PATH_PORT, NULL, NULL, NULL, NULL, NULL};
    args_porto[1] = id_mem_good;
    args_porto[2] = id_mem_conf;
    args_porto[3] = id_mem_port;
    args_porto[4] = id_mem_semop;
    sprintf(id_mem_good, "%d", sh_mem_id_good);
    sprintf(id_mem_conf, "%d", sh_mem_id_conf);
    sprintf(id_mem_port, "%d", sh_mem_id_port);
    sprintf(id_mem_semop, "%d", sh_mem_id_semaphore);
    /*eseguo la fork*/
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        /*---creation of the process port---- */
        switch (fork())
        {
        case -1:
            TEST_ERROR;
        case 0:
            ptr_shm_port[i].pid = getpid();
            ptr_shm_port[i].id_port = i;
            printf("----EXECVP DEL PORTO[%i] CON PID :[%i]-----\n", i, ptr_shm_port[i].pid);
            execvp(PATH_PORT, args_porto);
            perror("Execve error\n");
            exit(1);
        default:
            sleep(1);
        }
    }
    free(id_mem_conf);
    free(id_mem_good);
    free(id_mem_port);
    free(id_mem_semop);
}

void create_ship(struct ship *ptr_shm_ship, struct var_conf *ptr_shm_v_conf)
{

    /*-----argv da passare al porto-----*/
    char *args_ship[] = {PATH_SHIP, NULL, NULL, NULL, NULL, NULL, NULL};
    /*----assegno a questi valori l'id delle rispetive mem condivise----*/
    char *id_mem_good = malloc(sizeof(char));
    char *id_mem_conf = malloc(sizeof(char));
    char *id_mem_port = malloc(sizeof(char));
    char *id_mem_ship = malloc(sizeof(char));
    char *id_mem_semop = malloc(sizeof(char));
    /*----punto al puntatore----*/
    args_ship[1] = id_mem_good; // l'argomento di argv è il puntatore alla stringa che indica la memroia condivisa
    args_ship[2] = id_mem_conf;
    args_ship[3] = id_mem_port;
    args_ship[4] = id_mem_ship;
    args_ship[5] = id_mem_semop;
    /*----converto da intero a stringa-----*/
    sprintf(id_mem_good, "%d", sh_mem_id_good);
    sprintf(id_mem_conf, "%d", sh_mem_id_conf);
    sprintf(id_mem_port, "%d", sh_mem_id_port);
    sprintf(id_mem_ship, "%d", sh_mem_id_ship);
    sprintf(id_mem_semop, "%d", sh_mem_id_semaphore);
    /*----creo le navi----*/
    for (int j = 0; j < ptr_shm_v_conf->so_navi; j++)
    {
        switch (fork())
        {
        case -1:
            TEST_ERROR;
        case 0:
            /*-----inserisco il pid della nave per potermi riferire a quella-----*/
            ptr_shm_ship[j].pid = getpid();

            printf("----EXECVP DELLA %i^NAVE  CON PID:%i-----\n", j, getpid());

            execvp(PATH_SHIP, args_ship);
            perror("Execve error");
            exit(1);
        default:
            sleep(1);
        }
    }
    free(id_mem_good);
    free(id_mem_conf);
    free(id_mem_port);
    free(id_mem_ship);
    free(id_mem_semop);
}

void signalHandler(int signum)
{

    printf("Ricevuto segnale SIGINT. Eseguo la pulizia...\n");

    switch (signum)
    {
    case SIGINT:
    case SIGALRM:

        for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
        {
            if (shmctl(ptr_shm_port[i].id_shm_domanda, IPC_RMID, NULL) == -1)
            {
                printf("ERROR IN remove id_shm_domanda\n");
            }
            if (shmctl(ptr_shm_port[i].id_shm_offerta, IPC_RMID, NULL) == -1)
            {
                printf("ERROR IN remove shm offerta\n");
            }
            if (kill(ptr_shm_port[i].pid, SIGINT) == -1)
            {
                printf("ERROR IN KILL\n");
            }
        }
        printf("eliminato i porti\n");
        for (int i = 0; i < ptr_shm_v_conf->so_navi; i++)
        {

            if (kill(ptr_shm_ship[i].pid, SIGINT) != -1)
            {
                printf("segnale di terminazione inviato correttamente alla nave\n");
            }
        }
        if (shmctl(sh_mem_id_conf, IPC_RMID, NULL) == -1)
        {
            printf("ERROR IN shmctl_1 sh_mem_id_conf");
        }
        if (shmctl(sh_mem_id_good, IPC_RMID, NULL) == -1)
        {
            printf("ERROR IN shmctl_1 sh_mem_id_conf");
        }
        if (shmctl(sh_mem_id_port, IPC_RMID, NULL) == -1)
        {
            printf("ERROR IN shmctl_1 sh_mem_id_conf");
        }
        if (shmctl(sh_mem_id_ship, IPC_RMID, NULL))
        {
            printf("ERROR IN shmctl_1 sh_mem_id_conf");
        }
        if (shmctl(sh_mem_id_semaphore, IPC_RMID, NULL))
        {
            printf("ERROR IN SEMCTL SEM_ID");
        }
        if (semctl(sem_id, 0, IPC_RMID) == -1)
        {
            printf("ERROR IN SEMCTL SEM_ID");
        }
        if (semctl(sem_id_banchine, 0, IPC_RMID) == -1)
        {
            printf("ERROR IN SEMCTL SEM BANCHINE");
        }
        if (semctl(sem_id_shm, 0, IPC_RMID) == -1)
        {
            printf("ERROR IN SEMCTL SEM SHM");
        }
        break;
    }
    printf("eliminato tutto\n");
    exit(EXIT_SUCCESS);
}
