/*assegnare alle diverse struct i valori che devo configurare in configuration.c*/
#include "headership.h"
#include "configuration.h"
#include "math.h"
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

void ship_move_first_position(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int *id_porto, int id_ship)
{
    /*----utilities parameter----*/
    double tmp_shift, frac, intpart;
    struct timespec nanotime;
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
    // printf("array distanze creato: porto piu vicino a: [%f], secondo porto piu vicino a: [%f]\n", array_distance[indice_first_port], array_distance[0]);
    *id_porto = indice_first_port;
    tmp_shift = ((array_distance[indice_first_port]) / ((double)ptr_shm_v_conf->so_speed));
    nanotime.tv_sec = (int)tmp_shift;                             /*take seconds*/
    nanotime.tv_nsec = (tmp_shift - (int)tmp_shift) * 1000000000; /*take nanoseconds*/
    nanosleep(&nanotime, NULL);
    /*----ora devo modificare la x e la y della nave----*/
    ptr_shm_ship[id_ship].pos_ship.x = ptr_shm_port[indice_first_port].pos_porto.x;
    ptr_shm_ship[id_ship].pos_ship.y = ptr_shm_port[indice_first_port].pos_porto.y;
    // printf("nave:[%i]---->  x_aggiornato:%f, y_aggiornato:%f\n", id_ship, ptr_shm_ship[id_ship].pos_ship.x, ptr_shm_ship[id_ship].pos_ship.y);
    //  fflush(stdout);
    printf("------> nave %i nel porto con id: %i \n", id_ship, *id_porto);
    free(array_distance);
}
/*------funzione che mi permette di spostarmi nella mappa da porto a porto------*/
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int *id_porto, int id_ship)
{
    /*----utilities parameter----*/
    printf("nave: ------TEST sono nella ship move to\n");
    double tmp_shift;
    struct timespec nanotime;
    if (*id_porto >= ptr_shm_v_conf->so_porti - 1) // verificare la correttezza
    {
        tmp_shift = ((sqrt(pow(ptr_shm_port[0].pos_porto.x - ptr_shm_ship[id_ship].pos_ship.x, 2) + pow(ptr_shm_port[0].pos_porto.y - ptr_shm_ship[id_ship].pos_ship.y, 2))) / ((double)ptr_shm_v_conf->so_speed));
        nanotime.tv_sec = (int)tmp_shift;                             /*take seconds*/
        nanotime.tv_nsec = (tmp_shift - (int)tmp_shift) * 1000000000; /*take nanoseconds*/
        nanosleep(&nanotime, NULL);
        *id_porto = 0;
    }
    else
    {
        tmp_shift = ((DISTANZA_P_N_) / ((double)ptr_shm_v_conf->so_speed));
        nanotime.tv_sec = (int)tmp_shift;                             /*take seconds*/
        nanotime.tv_nsec = (tmp_shift - (int)tmp_shift) * 1000000000; /*take nanoseconds*/
        nanosleep(&nanotime, NULL);
        (*id_porto)++;
    }
    /*----ora devo modificare la x e la y della nave----*/
    ptr_shm_ship[id_ship].pos_ship.x = ptr_shm_port[*id_porto].pos_porto.x;
    ptr_shm_ship[id_ship].pos_ship.y = ptr_shm_port[*id_porto].pos_porto.y;
    printf("nave: posizione della nave[%i] aggiornata:(%f,%f), si trova al porto[%i]\n", id_ship, ptr_shm_ship[id_ship].pos_ship.x, ptr_shm_ship[id_ship].pos_ship.y, *id_porto);
}
void ship_expired_good(struct ship *ptr_shm_ship, struct var_conf *ptr_shm_v_conf, int id_ship, struct good *stiva){
    for(int i=0; i<ptr_shm_v_conf->so_merci; i++){
        if(stiva[i].life== ptr_shm_v_conf->days_real){
            stiva[i].lotti=0
        }
    }
}