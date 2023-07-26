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
struct port *porto;
struct ship *nave;
/*
 */
void ship_move_first_position(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship);
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship);
/*calcolo le distanze dall'origine*/
void ship_move_first_position(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship)
{
    /*----utilities parameter----*/
    double tmp;
    struct timespec *nanotime;
    /*array distanze lo utilizzo per selezionare il porto più vicino*/
    double *array_distance = malloc(sizeof(double) * ptr_shm_v_conf->so_porti);
    int indice_first_port = 0;
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        /*inserisco il valore assoluto*/
        array_distance[i] = sqrt(pow(ptr_shm_port[i].pos_porto.x - ptr_shm_ship[id_ship].pos_ship.x, 2) + pow(ptr_shm_port[i].pos_porto.y - ptr_shm_ship[id_ship].pos_ship.y, 2));
        // printf("distanza porto [%i] dalla nave[%i]: %f\n", id_porto, id_ship, array_distance[i]);
    }
    /*seleziono la distanza minore*/
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {                                                              // seleziono la distanza minore e l'indice del porto per la sua posizione
        if (array_distance[indice_first_port] > array_distance[i]) // controllare l'array delle distanze e in modo da ordinarlo e salvare gli indici nell'arrai degli indici
        {
            indice_first_port = i;
        }
    }
    // printf("array distanze creato: porto piu vicino:[%f],primo porto [%f]\n", array_distance[indice_first_port], array_distance[0]);
    id_porto = indice_first_port;
    tmp = ((array_distance[indice_first_port]) / ((double)ptr_shm_v_conf->so_speed));
    // printf("la nave farà un viaggio in mare che durerà %f secondi\n", tmp);
    //  nanotime->tv_nsec = (time_t)tmp;                /*casto a intero così prendo solo la prima parte decimale*/
    //  nanotime->tv_nsec = (long int)(tmp - (int)tmp); /*posso farlo per avere solo la parte decimale? */
    //  nanosleep(nanotime, NULL);
    /*----ora devo modificare la x e la y della nave----*/
    // printf("indice_first_port=[%i], id_ship=[%i]\n", indice_first_port, id_ship);
    ptr_shm_ship[id_ship].pos_ship.x = ptr_shm_port[indice_first_port].pos_porto.x;
    ptr_shm_ship[id_ship].pos_ship.y = ptr_shm_port[indice_first_port].pos_porto.y;
    // printf("ship_x_aggiornato: %f, ship_y_aggiornato:%f\n", ptr_shm_ship[id_ship].pos_ship.x, ptr_shm_ship[id_ship].pos_ship.y);
    free(array_distance);
}
/*------funzione che mi permette di spostarmi nella mappa da porto a porto------*/
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship)
{
    /*----utilities parameter----*/
    double tmp;
    struct timespec *nanotime;
    if (id_porto >= ptr_shm_v_conf->so_porti - 1)
    {
        tmp = ((sqrt(pow(ptr_shm_port[0].pos_porto.x - ptr_shm_ship[id_ship].pos_ship.x, 2) + pow(ptr_shm_port[0].pos_porto.y - ptr_shm_ship[id_ship].pos_ship.y, 2))) / ((double)ptr_shm_v_conf->so_speed)); // non ha senso è il caso in cui la nave sia all'ultimo porto

        printf("la nave farà un viaggio in mare che durerà %f secondi\n", tmp);
        // nanotime->tv_sec = (time_t)tmp;                 /*casto a intero così prendo solo la prima parte decimale*/
        // nanotime->tv_nsec = (long int)(tmp - (int)tmp); /*posso farlo per avere solo la parte decimale? */
        // nanosleep(nanotime, NULL);
        id_porto = 0;
    }
    else
    {
        tmp = ((DISTANZA_P_N_) / ((double)ptr_shm_v_conf->so_speed));
        printf("la nave farà un viaggio in mare che durerà %f secondi\n", tmp);
        // nanotime->tv_nsec = (time_t)tmp;                /*casto a intero così prendo solo la prima parte decimale*/
        // nanotime->tv_nsec = (long int)(tmp - (int)tmp); /*posso farlo per avere solo la parte decimale? */
        // nanosleep(nanotime, NULL);
        id_porto++;
    }
    /*----ora devo modificare la x e la y della nave----*/
    ptr_shm_ship[id_ship].pos_ship.x = ptr_shm_port[id_porto].pos_porto.x;
    ptr_shm_ship[id_ship].pos_ship.y = ptr_shm_port[id_porto].pos_porto.y;
    printf("posizione della nave[%i] aggiornata:(%f,%f) si trova al porto [%i]\n", id_ship, ptr_shm_ship[id_ship].pos_ship.x, ptr_shm_ship[id_ship].pos_ship.y, id_porto);
}

int main()
{
    srand(time(NULL));
    struct var_conf ptr_shm_v_conf;
    ptr_shm_v_conf.so_speed = 3;
    ptr_shm_v_conf.so_porti = 5;
    porto = malloc(sizeof(struct port) * 5);
    nave = malloc(sizeof(struct ship) * 3);
    // inserisco gli id:
    for (int i = 0; i < 5; i++)
    {
        porto[i].id_port = i;
        porto[i].pos_porto.x = rand() % 10;
        porto[i].pos_porto.y = rand() % 10;
        printf("la posizione del porto[%i] è:[%f,%f]\n", i, porto[i].pos_porto.x, porto[i].pos_porto.y);
    }
    for (int i = 0; i < 3; i++)
    {
        nave[i].id_ship = i;
        nave[i].pos_ship.x = rand() % 10;
        nave[i].pos_ship.y = rand() % 10;
        printf("la posizione della nave[%i] è:[%f,%f]\n", i, nave[i].pos_ship.x, nave[i].pos_ship.y);
    }
    port_sorting(&ptr_shm_v_conf, porto);
    for (int i = 0; i < 3; i++)
    {
        ship_move_first_position(nave, porto, &ptr_shm_v_conf, 0, nave[i].id_ship);
    }
    printf("finito la ship move della prima posizione\n");
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            ship_move_to(nave, porto, &ptr_shm_v_conf, i, nave[j].id_ship);
        }
    }
    free(porto);
    free(nave);
}