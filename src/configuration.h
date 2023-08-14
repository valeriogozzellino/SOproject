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
#define SO_DAYS ptr_shm_v_conf->so_days

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
    int so_banchine; /*nave non aspetta che la banchina sia libera*/
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
    int maelstorm;
    int storm;
    int swell;
};
/*-----definition of the ship's structure----- */
struct ship
{
    pid_t pid;                /*pi del processo*/
    int id_ship;              /*dentificativo della nave */
    struct position pos_ship; /*posizione della nave nella mappa*/
    double capacity;          /*capacità della nave in tonnellate*/
    int speed;                /*velocitàò della nave */
    int location;             /*posizione della naveper il dump, se in porto 1 o in mare 0 */
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

struct mymsg
{
    long mtype;
    char mtext[100];
};
/*funzione che mi apre il file */
void open_file(FILE **f_c);

/* ---funzione che legge dei valori da "file-configurazione" per inizializzare i parametri della struct var_conf
    che contiene le variabili di configurazione SO_...----- */
void find_val(struct var_conf *env_var); /*CORRETTA*/

/*-----inizializzo le var di configurazione in shared memory----*/
void sh_memory_v_conf(struct var_conf env_var, struct var_conf *ptr_shm_v_conf);

/*-----inizialization of merce shared memory----*/
void sh_memory_v_good(struct var_conf env_var, struct good *ptr_shm_good);
/*-----insert all caratteristic pf good in stiva-----*/
void set_good_ship(struct good *ptr_shm_good, struct good **stiva, struct var_conf env_var);
/*-----inizialization of port shared memory----*/
void sh_memory_v_porti(struct var_conf env_var, struct port *ptr_shm_port);
/*-----sorting of the port's distance from the origin of the map and assignement of id_port------*/
void port_sorting(struct var_conf *ptr_shm_v_conf, struct port *ptr_shm_port); /*CORRETTA*/
/*-----inizialization of ship shared memory----*/
void sh_memory_v_ship(struct var_conf env_var, struct ship *ptr_shm_ship);
/*-----inizializzo i valori dei semafori nel master-------*/
void load_val_semaphor(int sem_id_banchine, int sem_id_shm, int sem_id, int *ptr_shm_semaphore, struct var_conf *ptr_shm_v_conf);

#endif /* CONFIGURATION_H*/