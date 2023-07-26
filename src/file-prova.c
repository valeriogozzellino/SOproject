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
/*
 */
double distance_from_origin(struct port *porto);
void swap_ports(struct port *a, struct port *b);
void port_sorting(struct var_conf *ptr_shm_v_conf, struct port *ptr_shm_port);

void ship_move_first_position(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship);
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship);
/*calcolo le distanze dall'origine*/
double distance_from_origin(struct port *porto)
{
    return sqrt(pow(porto->pos_porto.x, 2) + pow(porto->pos_porto.y, 2));
}

// Metodo per lo scambio di due porti all'interno dell'array
void swap_ports(struct port *a, struct port *b)
{
    struct port tmp = *a;
    *a = *b;
    *b = tmp;
}

// Metodo per l'ordinamento dell'array `ptr_shm_port` in base alla distanza dal punto (0,0)
void port_sorting(struct var_conf *ptr_shm_v_conf, struct port *ptr_shm_port)
{
    int length = ptr_shm_v_conf->so_porti;
    int sorted = 0;

    while (!sorted)
    {
        sorted = 1;
        for (int j = 0; j < length - 1; j++)
        {
            double dist1 = distance_from_origin(&ptr_shm_port[j]);
            double dist2 = distance_from_origin(&ptr_shm_port[j + 1]);

            if (dist1 > dist2)
            {
                swap_ports(&ptr_shm_port[j], &ptr_shm_port[j + 1]);
                sorted = 0;
            }
        }
        length--;
    }

    // Ora l'array è ordinato, puoi stampare l'output o fare altre operazioni
}
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
        printf("distanza porti dalla nave: %f\n", array_distance[i]);
    }
    /*seleziono la distanza minore*/
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {                                                              // seleziono la distanza minore e l'indice del porto per la sua posizione
        if (array_distance[indice_first_port] > array_distance[i]) // controllare l'array delle distanze e in modo da ordinarlo e salvare gli indici nell'arrai degli indici
        {
            indice_first_port = i;
        }
    }
    printf("array distanze creato: porto piu vicino:[%f],primo porto [%f]\n", array_distance[indice_first_port], array_distance[0]);
    id_porto = indice_first_port;
    tmp = ((array_distance[indice_first_port]) / ((double)ptr_shm_v_conf->so_speed));
    printf("la nave farà un viaggio in mare che durerà %f secondi\n", tmp);
    nanotime->tv_nsec = (time_t)tmp;                /*casto a intero così prendo solo la prima parte decimale*/
    nanotime->tv_nsec = (long int)(tmp - (int)tmp); /*posso farlo per avere solo la parte decimale? */
    nanosleep(nanotime, NULL);
    /*----ora devo modificare la x e la y della nave----*/
    ptr_shm_ship[id_ship].pos_ship.x = ptr_shm_port[indice_first_port].pos_porto.x;
    ptr_shm_ship[id_ship].pos_ship.y = ptr_shm_port[indice_first_port].pos_porto.y;
    printf("x_aggiornato:%f, y_aggiornato:%f", ptr_shm_ship[id_ship].pos_ship.x, ptr_shm_ship[id_ship].pos_ship.y);
    free(array_distance);
}
/*------funzione che mi permette di spostarmi nella mappa da porto a porto------*/
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int id_porto, int id_ship)
{
    /*----utilities parameter----*/
    double tmp;
    struct timespec *nanotime;
    if (id_porto != ptr_shm_v_conf->so_porti - 1)
    {
        tmp = ((DISTANZA_P_N_) / ((double)ptr_shm_v_conf->so_speed));
        printf("la nave farà un viaggio in mare che durerà %f secondi\n", tmp);
        nanotime->tv_sec = (time_t)tmp;                 /*casto a intero così prendo solo la prima parte decimale*/
        nanotime->tv_nsec = (long int)(tmp - (int)tmp); /*posso farlo per avere solo la parte decimale? */
        nanosleep(nanotime, NULL);
        id_porto++;
    }
    else
    {
        tmp = ((DISTANZA_P_N_) / ((double)ptr_shm_v_conf->so_speed));
        printf("la nave farà un viaggio in mare che durerà %f secondi\n", tmp);
        nanotime->tv_nsec = (time_t)tmp;                /*casto a intero così prendo solo la prima parte decimale*/
        nanotime->tv_nsec = (long int)(tmp - (int)tmp); /*posso farlo per avere solo la parte decimale? */
        nanosleep(nanotime, NULL);
        id_porto = 0;
    }
    /*----ora devo modificare la x e la y della nave----*/
    ptr_shm_ship[id_ship].pos_ship.x = ptr_shm_port[id_porto].pos_porto.x;
    ptr_shm_ship[id_ship].pos_ship.y = ptr_shm_port[id_porto].pos_porto.y;
    printf("posizione della nave[%i] aggiornata:(%f,%f)", id_ship, ptr_shm_ship[id_ship].pos_ship.x, ptr_shm_ship[id_ship].pos_ship.y);
}

int main()
{
    srand(0);
    int type_offered, type_asked;
    struct var_conf ptr_shm_v_conf;
    ptr_shm_v_conf.so_speed = 3;
    ptr_shm_v_conf.so_porti = 5;
    porto = malloc(sizeof(struct port) * 5);
    // inserisco gli id:
    for (int i = 0; i < 5; i++)
    {
        porto[i].id_port = i;
        porto[i].pos_porto.x = rand() % 10;
        porto[i].pos_porto.y = rand() % 10;
    }
    port_sorting(&ptr_shm_v_conf, porto);
    for (int i = 0; i < 5; i++)
    {
        printf("porti[%i], \n", porto[i].id_port);
    }
    free(porto);
}