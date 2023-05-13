#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <math.h>
#define PATH_PORT "./port"
#define PATH_SHIP "./ship"
#define RD_T0_GO 1         /*processi pronti a partire*/
#define START_SIMULATION 0 /*semaphore*/
#define DISTANZA_P_N_ sqrt(pow(ptr_shm_port[id_porto + 1].pos_porto.x - ptr_shm_ship[id_ship].pos_ship.x, 2) + pow(ptr_shm_port[id_porto + 1].pos_porto.y - ptr_shm_ship[id_ship].pos_ship.y, 2))
#define TEST_ERROR                                                \
    if (errno)                                                    \
    {                                                             \
        fprintf(stderr, "errore: in %s , line: %i , errore: %s ", \
                __FILE__, __LINE__, strerror(errno));             \
    };

struct var_conf
{
    int so_days;
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
};

struct good
{
    int id;   /*numero che identifica la merce*/
    int size; /*size determinata tra 1<x<SO_SIZE generata nel master e assegnata ai porti, ogni porto diversa*/
    int lotti;
    int life;         /*cutdown alla scadenza, un intero che utilizzo come allarme per segnalare che una merce è scaduta*/
    int type_offered; // uguale indipendente a seconda del porto su cui mi trovo va a specificare il numero dei tipidi merce offerta
    int type_asked;
    int ritirata;   /*settato a 1 se una nave ha preo questo carico*/
    int consegnata; /*settato a 1 se la merce è consegnata al porto*/
};

/*----position of object in the map-----*/
struct position
{
    double x;
    double y;
};

/*-----definition of the ship's structure----- */
struct ship
{
    pid_t pid; /*valore del processo da assegnare alla nave*/
    int id_ship;
    struct position pos_ship; /*posizione della nave nella mappa*/
    int capacity;             /*capacità della nave in tonnellate*/
    int speed;                /*(chiedere se posso assegnare in un header file)*/
    int location;             /*mi serve per il dump giornaliero, 0= nel mare, 1= in porto*/
};

/*----definition of port's structure-----*/
struct port
{
    pid_t pid; /*processo che gli assegno*/
    int id_port;
    struct position pos_porto; /*poizione del porto nella mappa*/
    int n_banchine;            /*capire bene come determinare le banchine, sono dei semafori che permettono ad una barca di accedere o no*/
    int fill;
    int id_shm_offerta;
    int id_shm_domanda;
    int g_received; /*merce ricevuta */
    int g_send;     /*merce inviata */
};
// utilizzata per inviare i messaggi
struct mymsg
{
    long mtype;
    char mtext[100];
};
/*funzione che mi apre il file */
void open_file(FILE **f_c);

/* ---funzione che legge dei valori da "file-configurazione" per inizializzare i parametri della struct var_conf
    che contiene le variabili di configurazione SO_...----- */
void find_val(struct var_conf *env_var);

/*-----inizializzo le var di configurazione in shared memory----*/
void sh_memory_v_conf(struct var_conf env_var, struct var_conf *ptr_shm_v_conf);

/*-----inizialization of merce shared memory----*/
void sh_memory_v_good(struct var_conf env_var, struct good *ptr_shm_good);

/*-----inizialization of port shared memory----*/
void sh_memory_v_porti(struct var_conf env_var, struct port *ptr_shm_port);
/*-----sorting of the port's distance from the origin of the map and assignement of id_port------*/
void port_sorting(struct var_conf *ptr_shm_v_conf, struct port *ptr_shm_port);
/*-----inizialization of ship shared memory----*/
void sh_memory_v_ship(struct var_conf env_var, struct ship *ptr_shm_ship);
/*-----inizializzo i valori dei semafori nel master-------*/
void load_val_semaphor(int sem_id_banchine, int sem_id_shm, int sem_id, int *ptr_shm_semaphore, struct var_conf *ptr_shm_v_conf);

#endif // CONFIGURATION_