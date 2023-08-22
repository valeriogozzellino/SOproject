/*assegnare alle diverse struct i valori che devo configurare in configuration.c*/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include "headership.h"
#include "configuration.h"
#include "math.h"

void ship_move_first_position(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int *id_porto, int id_ship)
{
    /*----utilities parameter----*/
    double tmp_shift, frac, intpart;
    struct timespec ship_movement;
    double *array_distance = malloc(sizeof(double) * ptr_shm_v_conf->so_porti);
    int *array_indici = malloc(sizeof(int) * ptr_shm_v_conf->so_porti);
    int indice_first_port = 0;
    int i;
    for (i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        /*inserisco il valore assoluto*/
        array_indici[i] = ptr_shm_port[i].id_port;
        array_distance[i] = sqrt(pow(ptr_shm_port[i].pos_porto.x - ptr_shm_ship[id_ship].pos_ship.x, 2) + pow(ptr_shm_port[i].pos_porto.y - ptr_shm_ship[id_ship].pos_ship.y, 2));
    }
    /*seleziono la distanza minore*/
    for (i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        if (array_distance[indice_first_port] > array_distance[i])
        {
            indice_first_port = i;
        }
    }
    *id_porto = array_indici[indice_first_port];
    tmp_shift = array_distance[indice_first_port] / (double)ptr_shm_v_conf->so_speed;
    ship_movement.tv_sec = (time_t)tmp_shift;
    ship_movement.tv_nsec = (tmp_shift - ship_movement.tv_sec) * 1e9; /*take nanoseconds*/
    nanosleep(&ship_movement, NULL);
    /*----ora devo modificare la x e la y della nave----*/
    ptr_shm_ship[id_ship].pos_ship.x = ptr_shm_port[indice_first_port].pos_porto.x;
    ptr_shm_ship[id_ship].pos_ship.y = ptr_shm_port[indice_first_port].pos_porto.y;

    //printf("------> SHIP %i nel porto con id: %i \n", id_ship, *id_porto);
    free(array_distance);
}
/*------funzione che mi permette di spostarmi nella mappa da porto a porto------*/
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int *id_porto, int id_ship)
{
    /*----utilities parameter----*/
    double tmp_shift;
    struct timespec nanotime;
    if (*id_porto >= ptr_shm_v_conf->so_porti - 1)
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
}
void ship_expired_good(struct ship *ptr_shm_ship, struct var_conf *ptr_shm_v_conf, struct good *ptr_shm_good, int id_ship, struct good **stiva)
{
    int i, j;
    for (i = 0; i <= ptr_shm_v_conf->days_real; i++)
    {
        for (j = 0; j < ptr_shm_v_conf->so_merci; j++)
        {

            if (stiva[i][j].life <= ptr_shm_v_conf->days_real && stiva[i][j].lotti > 0)
            {
                ptr_shm_good[j].recap.ship_expired += stiva[i][j].lotti;
                stiva[i][j].lotti = 0;
            }
        }
    }
}
void sendAttackMessage(int portMessageQueue, const char *message)
{
    struct Message msg;
    msg.messageType = 1; // Set the message type (can be any positive integer)
    strncpy(msg.messageText, message, MAX_MESSAGE_SIZE);

    if (msgsnd(portMessageQueue, &msg, sizeof(msg.messageText), IPC_NOWAIT) == -1)
    {
        perror("Failed to send message to port");
        exit(1);
    }
}