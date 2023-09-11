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
#define NUM_PROCESSI (env_var.so_navi + env_var.so_porti + 4)

/*--------variables---------*/
struct var_conf env_var;
struct good merce;
struct port porto;
struct var_conf *ptr_shm_v_conf;
struct good *ptr_shm_good;
struct port *ptr_shm_port;
struct ship *ptr_shm_ship;
struct sembuf sops;
int *ptr_shm_semaphore;
int sh_mem_id_good, sh_mem_id_conf, sh_mem_id_port, sh_mem_id_ship, sh_mem_id_semaphore;
int sem_id_banchine, sem_id_shm, sem_id;
int status;
int active_process;
int days_real, i;
int *control_process;

/*------------prototipi di funzioni-----------*/
void create_port(struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf);
void create_ship(struct ship *ptr_shm_ship, struct var_conf *ptr_shm_v_conf);
void create_storm(struct var_conf *ptr_shm_v_conf);
void create_dump(struct var_conf *ptr_shm_v_conf);
/*---inizializza i semafori----*/
void signalHandler(int signum);
/*----------MAIN-----------*/
int main()
{
    signal(SIGINT, signalHandler);
    signal(SIGALRM, signalHandler);
    days_real = 0;
    control_process = malloc(sizeof(int) * 4);
    int active_ship;
    srand(time(NULL));

    find_val(&env_var);
    /*---------create shared memory id----------*/
    if ((sh_mem_id_conf = shmget(IPC_PRIVATE, sizeof(struct var_conf), 0600)) < 0)
    {
        TEST_ERROR;
    }
    if ((sh_mem_id_good = shmget(IPC_PRIVATE, (sizeof(struct good) * env_var.so_merci), 0600)) < 0)
    {
        TEST_ERROR;
    }
    if ((sh_mem_id_port = shmget(IPC_PRIVATE, (sizeof(struct port) * env_var.so_porti), 0600)) < 0)
    {
        TEST_ERROR;
    }
    if ((sh_mem_id_ship = shmget(IPC_PRIVATE, (sizeof(struct ship) * env_var.so_navi), 0600)) < 0)
    {
        TEST_ERROR;
    }
    if ((sh_mem_id_semaphore = shmget(IPC_PRIVATE, (sizeof(int) * 3), 0600)) < 0)
    {
        TEST_ERROR;
    }

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

    /*-----creazione dei  porti-----*/
    create_port(ptr_shm_port, ptr_shm_v_conf);
    /*-----creazione delle navi-----*/
    create_ship(ptr_shm_ship, ptr_shm_v_conf);
    /*-----create storm------------*/
    create_storm(ptr_shm_v_conf);
    /*-----create dump-------------*/
    create_dump(ptr_shm_v_conf);

    /*----ALL PROCESS CREATED-----*/
    printf("----MASTER: ALL PX CREATE: %i----\n", NUM_PROCESSI);
    sops.sem_flg = 0;
    sops.sem_num = RD_T0_GO;
    sops.sem_op = -NUM_PROCESSI;
    semop(sem_id, &sops, 1);
    /*----leave the resources to start simulation-----*/
    printf("----MASTER: 'GOOO'----\n");
    sops.sem_flg = 0;
    sops.sem_num = START_SIMULATION;
    sops.sem_op = NUM_PROCESSI;
    semop(sem_id, &sops, 1);
    printf("----MASTER: I ISSUED THE PX----\n");
    /*----START time simulation, master check the px active -----*/
    alarm(env_var.so_days);
    printf("----MASTER: ALARM ACTIVETED----\n");
    active_process = NUM_PROCESSI;
    while (active_process > 0)
    {
        sleep(1); /*sleep one day*/
        printf("----MASTER: IT'S BEEN A DAY----\n");
        ptr_shm_v_conf->days_real++;
        active_ship = 0;
        for (i = 0; i < env_var.so_navi; i++)
        {

            if (ptr_shm_ship[i].pid <= 0)
            {
                active_process--;
            }
            else
            {
                active_ship++;
            }
        }
        if (active_ship == 0)
        {
            signalHandler(2);
        }
        for (i = 0; i < env_var.so_porti; i++)
        {
            if (ptr_shm_port[i].pid <= 0)
            {
                active_process--;
            }
        }
    }

    /*--------TERMINAZIONE DELLA SIMULAZIONE-----------------*/
    printf("----MASTER: I'M ABOUT TO DELETE----\n");

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
    printf("----MASTER: I'M DELETE ALL----\n");
    return 0;
}

void create_port(struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf)
{
    int i;
    char *args_porto[] = {PATH_PORT, NULL, NULL, NULL, NULL, NULL};
    char *id_mem_good = malloc(sizeof(char));
    char *id_mem_conf = malloc(sizeof(char));
    char *id_mem_port = malloc(sizeof(char));
    char *id_mem_semop = malloc(sizeof(char));
    args_porto[1] = id_mem_good;
    args_porto[2] = id_mem_conf;
    args_porto[3] = id_mem_port;
    args_porto[4] = id_mem_semop;
    sprintf(id_mem_good, "%d", sh_mem_id_good);
    sprintf(id_mem_conf, "%d", sh_mem_id_conf);
    sprintf(id_mem_port, "%d", sh_mem_id_port);
    sprintf(id_mem_semop, "%d", sh_mem_id_semaphore);
    /*eseguo la fork*/
    for (i = 0; i < ptr_shm_v_conf->so_porti; i++)
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
        }
    }
    free(id_mem_conf);
    free(id_mem_good);
    free(id_mem_port);
    free(id_mem_semop);
}

void create_ship(struct ship *ptr_shm_ship, struct var_conf *ptr_shm_v_conf)
{

    int j;
    char *args_ship[] = {PATH_SHIP, NULL, NULL, NULL, NULL, NULL, NULL};
    char *id_mem_good = malloc(sizeof(char));
    char *id_mem_conf = malloc(sizeof(char));
    char *id_mem_port = malloc(sizeof(char));
    char *id_mem_ship = malloc(sizeof(char));
    char *id_mem_semop = malloc(sizeof(char));
    args_ship[1] = id_mem_good;
    args_ship[2] = id_mem_conf;
    args_ship[3] = id_mem_port;
    args_ship[4] = id_mem_ship;
    args_ship[5] = id_mem_semop;
    sprintf(id_mem_good, "%d", sh_mem_id_good);
    sprintf(id_mem_conf, "%d", sh_mem_id_conf);
    sprintf(id_mem_port, "%d", sh_mem_id_port);
    sprintf(id_mem_ship, "%d", sh_mem_id_ship);
    sprintf(id_mem_semop, "%d", sh_mem_id_semaphore);
    for (j = 0; j < ptr_shm_v_conf->so_navi; j++)
    {
        switch (fork())
        {
        case -1:
            TEST_ERROR;
        case 0:
            ptr_shm_ship[j].pid = getpid();

            printf("----EXECVP OF SHIP %i^ WITH PID:%i-----\n", j, getpid());

            execvp(PATH_SHIP, args_ship);
            perror("Execve error\n");
            exit(1);
        default:
        }
    }
    free(id_mem_good);
    free(id_mem_conf);
    free(id_mem_port);
    free(id_mem_ship);
    free(id_mem_semop);
}
void create_dump(struct var_conf *ptr_shm_v_conf)
{

    char *args_dump[] = {PATH_SHIP, NULL, NULL, NULL, NULL, NULL, NULL};
    char *id_mem_good = malloc(sizeof(char));
    char *id_mem_conf = malloc(sizeof(char));
    char *id_mem_port = malloc(sizeof(char));
    char *id_mem_ship = malloc(sizeof(char));
    char *id_mem_semop = malloc(sizeof(char));
    args_dump[1] = id_mem_good;
    args_dump[2] = id_mem_conf;
    args_dump[3] = id_mem_port;
    args_dump[4] = id_mem_ship;
    args_dump[5] = id_mem_semop;
    sprintf(id_mem_good, "%d", sh_mem_id_good);
    sprintf(id_mem_conf, "%d", sh_mem_id_conf);
    sprintf(id_mem_port, "%d", sh_mem_id_port);
    sprintf(id_mem_ship, "%d", sh_mem_id_ship);
    sprintf(id_mem_semop, "%d", sh_mem_id_semaphore);

    switch (fork())
    {
    case -1:
        TEST_ERROR;
    case 0:
        control_process[3] = getpid();
        execvp(PATH_DUMP, args_dump);
        perror("Execve error\n");
        exit(1);
    default:
    }
    free(id_mem_good);
    free(id_mem_conf);
    free(id_mem_port);
    free(id_mem_ship);
}

void create_storm(struct var_conf *ptr_shm_v_conf)
{
    char *args_storm[] = {PATH_SHIP, NULL, NULL, NULL, NULL, NULL};
    char *id_mem_conf = malloc(sizeof(char));
    char *id_mem_port = malloc(sizeof(char));
    char *id_mem_ship = malloc(sizeof(char));
    char *id_mem_semop = malloc(sizeof(char));
    args_storm[1] = id_mem_conf;
    args_storm[2] = id_mem_port;
    args_storm[3] = id_mem_ship;
    args_storm[4] = id_mem_semop;
    sprintf(id_mem_conf, "%d", sh_mem_id_conf);
    sprintf(id_mem_port, "%d", sh_mem_id_port);
    sprintf(id_mem_ship, "%d", sh_mem_id_ship);
    sprintf(id_mem_semop, "%d", sh_mem_id_semaphore);
    switch (fork())
    {
    case -1:
        TEST_ERROR;
    case 0:
        control_process[0] = getpid();
        execvp(PATH_STORM, args_storm);
        perror("Execve error\n");
        exit(1);
    default:
    }
    switch (fork())
    {
    case -1:
        TEST_ERROR;
    case 0:
        control_process[1] = getpid();
        execvp(PATH_SWELL, args_storm);
        perror("Execve error\n");
        exit(1);
    default:
    }
    switch (fork())
    {
    case -1:
        TEST_ERROR;
    case 0:
        control_process[2] = getpid();
        execvp(PATH_MAELSTROM, args_storm);
        perror("Execve error\n");
        exit(1);
    default:
    }

    free(id_mem_conf);
    free(id_mem_port);
    free(id_mem_ship);
    free(id_mem_semop);
}
void signalHandler(int signum)
{
    int i;
    printf("------Ricevuto segnale al MASTER. Eseguo la pulizia-------\n");

    switch (signum)
    {
    case SIGINT:
    case SIGALRM:

        for (i = 0; i < ptr_shm_v_conf->so_porti; i++)
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
        printf("MASTER: eliminato i porti\n");
        for (i = 0; i < ptr_shm_v_conf->so_navi; i++)
        {

            if (kill(ptr_shm_ship[i].pid, SIGINT) != -1)
            {
                printf("MASTER: segnale di terminazione inviato correttamente alla nave\n");
            }
        }
        printf("MASTER: elimino i processi di controllo\n");
        for (i = 0; i < 4; i++)
        {

            if (kill(control_process[i], SIGINT) != -1)
            {
                printf("MASTER: segnale di terminazione inviato correttamente ai processi di controllo\n");
            }
        }
        printf("MASTER: ATTENDO LA TERMINAZIONE DEI PROCESSI PRIMA DI ELIMINARE LE MEMORIA CONDIVISE\n");
        while (wait(&status) != -1)
        {
            printf("MASTER: attendo terminazione dei processi\n");
        }
        if (shmctl(sh_mem_id_conf, IPC_RMID, NULL) == -1)
        {
            printf("ERROR IN shmctl_1 sh_mem_id_conf\n");
        }
        if (shmctl(sh_mem_id_good, IPC_RMID, NULL) == -1)
        {
            printf("ERROR IN shmctl_1 sh_mem_id_conf\n");
        }
        if (shmctl(sh_mem_id_port, IPC_RMID, NULL) == -1)
        {
            printf("ERROR IN shmctl_1 sh_mem_id_conf\n");
        }
        if (shmctl(sh_mem_id_ship, IPC_RMID, NULL))
        {
            printf("ERROR IN shmctl_1 sh_mem_id_conf\n");
        }
        if (shmctl(sh_mem_id_semaphore, IPC_RMID, NULL))
        {
            printf("ERROR IN SEMCTL SEM_ID\n");
        }
        if (semctl(sem_id, 0, IPC_RMID) == -1)
        {
            printf("ERROR IN SEMCTL SEM_ID\n");
        }
        if (semctl(sem_id_banchine, 0, IPC_RMID) == -1)
        {
            printf("ERROR IN SEMCTL SEM BANCHINE\n");
        }
        if (semctl(sem_id_shm, 0, IPC_RMID) == -1)
        {
            printf("ERROR IN SEMCTL SEM SHM\n");
        }
        break;
    }
    printf("MASTER: eliminato tutto\n");
    exit(EXIT_SUCCESS);
}
