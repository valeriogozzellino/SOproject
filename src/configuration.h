#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <math.h>
#define PATH_PORT "bin/port"
#define PATH_SHIP "bin/ship"
#define PATH_STORM "bin/storm_duration"
#define PATH_MAELSTROM "bin/maelstorm"
#define PATH_SWELL "bin/swell_duration"
#define PATH_DUMP "bin/dump"
#define RD_T0_GO 1         /*processi pronti a partire*/
#define START_SIMULATION 0 /*semaphore*/
#define SO_DAYS 10

#define DISTANZA_P_N_ sqrt(pow(ptr_shm_port[*id_porto + 1].pos_porto.x - ptr_shm_ship[id_ship].pos_ship.x, 2) + pow(ptr_shm_port[*id_porto + 1].pos_porto.y - ptr_shm_ship[id_ship].pos_ship.y, 2))
#define TEST_ERROR                                                \
    if (errno)                                                    \
    {                                                             \
        fprintf(stderr, "errore: in %s , line: %i , errore: %s ", \
                __FILE__, __LINE__, strerror(errno));             \
    };

struct var_conf
{
    int so_days;
    int days_real;
    int so_navi;
    int so_porti;
    int so_merci;
    int so_size;
    int so_min_vita;
    int so_max_vita;
    double so_lato;
    int so_speed;
    int so_loadspeed;
    int so_fill;
    int so_banchine;
    int so_capacity;
    double so_maelstorm;
    double so_swell_duration;
    double so_storm_duration;
};

struct dump
{                     /* merce scaduta in nave o nel porto, a seconda di quale dei due parametri è settato a 1*/
    int ship_expired; /*scaduta sulla nave*/
    int port_expired; /*scaduta nel porto*/
    int ritirata;     /*settato a 1 se una nave ha preo questo carico*/
    int consegnata;   /*settato a 1 se la merce è consegnata al porto*/
    double size_created;
};
struct good
{
    int id;            /*numero che identifica la merce*/
    int size;          /*size determinata tra 1<x<SO_SIZE generata nel master e assegnata ai porti, ogni porto diversa*/
    int lotti;         /*numero di lotti che vengono resi disponibili giornalmente*/
    int life;          /*la scadenza della merce sarà fatta sommando i giorni che sono passati con quelli in cui non è scaduta ad esempio se si conserva 10 giorni... guardare README*/
    struct dump recap; /*struct con tutte le info per la gestione del dump giornaliero*/
};

/*----position of object in the map-----*/
struct position
{
    double x;
    double y;
};

struct sink
{
    int maelstorm; /*count the number of ship sink by maelstorm*/
    int storm;     /*count the numberof ship hit by storm*/
    int swell;     /*count the number of ship slowdown */
};
/*-----definition of the ship's structure----- */
struct ship
{
    pid_t pid;                /*pi del processo*/
    int id_ship;              /*dentificativo della nave */
    struct position pos_ship; /*posizione della nave nella mappa*/
    double capacity;          /*capacità della nave in tonnellate*/
    int speed;                /*velocitàò della nave */
    int location;             /*posizione della nave per il dump, se in porto 1 o in mare 0 */
    int in_exchange;          /*settato a 1 se la nave sta facendo scambi di merce con il porto, altrimenti 0*/
    int sink_check;
    struct sink sink_type; /*0 affondata e 1 no*/
};

/*----definition of port's structure-----*/
struct port
{
    pid_t pid;                 /*pid del processo*/
    int id_port;               /*identificativo del porto*/
    struct position pos_porto; /*poizione del porto nella mappa*/
    int n_banchine;            /*banchine di ogni porto*/
    int fill;                  /*numero massimo di merce in offerta e domanda di tutti i porti*/
    int id_shm_offerta;        /* id della shared memory della domanda di merci*/
    int id_shm_domanda;        /* id della shared memory della domanda di merci*/
    int n_type_offered;        /* numero di tipi di merce offerti alle navi*/
    int n_type_asked;          /* numero di tipi di merce offerti alle navi*/
    int g_received;            /*merce ricevuta */
    int g_send;                /*merce inviata */
    int pid_ship;              /*pid della nave che sta facendo scambi se !=0 utile per swellduration*/
    int banchine_occupate;     /* banchine occupate al momento del dump giornaliero*/
    int message_queue_key;
};

struct Message
{
    long messageType;
    char messageText[100];
};
/**
 * find_val: This function reads and extracts configuration values from a file.
 * It opens a configuration file, parses lines containing variable assignments, and updates the provided var_conf structure with the extracted values.
 * It dynamically interprets variables' names and assigns their corresponding values.
 * This function is essential for dynamically loading configuration parameters for the simulation.
 */
void find_val(struct var_conf *env_var);
/**
 * open_file: A helper function that opens a configuration file in read mode.
 * It's used by the find_val function to access the configuration file for extracting variable values.
 * This function ensures proper file handling and error reporting.
 */
void open_file(FILE **f_c);

/**
 * sh_memory_v_conf: This function initializes shared memory variables for configuration parameters.
 * It copies the values from the provided env_var structure to a shared memory ptr_shm_v_conf structure.
 * This is a critical step to ensure that all processes accessing shared memory have consistent configuration data for the simulation.
 */
void sh_memory_v_conf(struct var_conf env_var, struct var_conf *ptr_shm_v_conf);

/**
 * sh_memory_v_good: This function initializes shared memory variables for goods.
 * It populates the ptr_shm_good array with goods' attributes such as ID, size, and lifespan.
 * The function generates random values within specified ranges and assigns them to each good, ensuring variability among goods.
 */
void sh_memory_v_good(struct var_conf env_var, struct good *ptr_shm_good);
/**
 * set_good_ship: This function initializes shared memory variables for goods on ships.
 * It copies the attributes of goods stored in the ptr_shm_good array to the stiva array, which represents goods on ships across different days.
 * It ensures that goods' attributes are synchronized between different data structures.
 */
void set_good_ship(struct good *ptr_shm_good, struct good **stiva, struct var_conf env_var);
/**
 * This function initializes shared memory variables for ports.
 * It populates the ptr_shm_port array with port attributes such as the number of docks and fill percentage.
 * It also assigns positions to each port, distributing them across specified coordinates or random positions within a range.
 */
void sh_memory_v_porti(struct var_conf env_var, struct port *ptr_shm_port);
/**
 * port_sorting: This function sorts ports based on their distance from the origin using the bubble sort algorithm.
 * It reorders the ptr_shm_port array, placing ports with shorter distances earlier in the array.
 * This is useful for optimizing the order in which ships visit ports.
 */
void port_sorting(struct var_conf *ptr_shm_v_conf, struct port *ptr_shm_port); /*CORRETTA*/
/**
 * sh_memory_v_ship: This function initializes shared memory variables for ships.
 * It assigns attributes such as capacity, speed, and positions to each ship within the ptr_shm_ship array.
 * It also generates random positions within specified ranges for each ship.
 */
void sh_memory_v_ship(struct var_conf env_var, struct ship *ptr_shm_ship);
/**
 * load_val_semaphor: This function initializes semaphore values for synchronization.
 * It sets initial values for semaphores associated with dock access, shared memory access, and simulation start.
 * It ensures proper synchronization and coordination among different processes accessing shared resources in the simulation.
 */
void load_val_semaphor(int sem_id_banchine, int sem_id_shm, int sem_id, int *ptr_shm_semaphore, struct var_conf *ptr_shm_v_conf);

#endif /* CONFIGURATION_H*/