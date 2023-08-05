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
void prova_scambio()
{

    if (semop(ptr_shm_sem[1], &sops, 1) != -1)
    { /*accedo alla memoria condivisa per lo scambio della merce*/
        /*controllo in memroia condivisa se il porto ha richiesta/domanda*/
        printf("SHIP: la nave %i, è attaccata al porto %i, sta per effettuare scambi di merce\n", id_ship, *id_porto);
        // mi sono collegato alla memoria condivisa del porto
        offerta_porto = shmat(ptr_shm_port[*id_porto].id_shm_offerta, NULL, 0600);
        domanda_porto = shmat(ptr_shm_port[*id_porto].id_shm_domanda, NULL, 0600);
        // verifico la domanda, poi prendo la domanda per svuotare la nave
        merce_scambiata = 0;
        if (ptr_shm_ship[id_ship].capacity != 0) // VERIFICO CHE LA MERCER ABBIA ANCORA SPAZIO IN STIVA
        {
            for (int j = 0; (j < ptr_shm_port[*id_porto].n_type_offered) || (ptr_shm_ship[id_ship].capacity == ptr_shm_v_conf->so_capacity); j++)
            {
                for (int i = 0; i < ptr_shm_v_conf->so_merci; i++)
                {
                    if (offerta_porto[j].id == stiva[i].id)
                    {
                        while ((ptr_shm_ship[id_ship].capacity != ptr_shm_v_conf->so_capacity) && (offerta_porto[j].id > 0))
                        {
                            offerta_porto[j].lotti--;
                            stiva[i].lotti++;
                            ptr_shm_ship[id_ship].capacity++;
                            ptr_shm_port[*id_porto].g_send++;
                            merce_scambiata = merce_scambiata + stiva[i].size;
                        }
                    }
                }
            }
        }
        for (int j = 0; j < ptr_shm_v_conf->so_merci; j++)
        {
            for (int z = 0; z < ptr_shm_port[*id_porto].n_type_asked; z++)
            {
                if (stiva[j].id == domanda_porto[z].id)
                {
                    printf("la nave %i sta soddifando la domanda del porto %i, la domanda  della merce [id= %i]è %i\n", id_ship, *id_porto, stiva[j].id, domanda_porto[z].lotti);
                    while ((stiva[z].lotti > 0) && (domanda_porto[z].lotti > 0))
                    {
                        stiva[z].lotti--;
                        domanda_porto[z].lotti--;
                        ptr_shm_ship[id_ship].capacity--;
                        printf("sto decrementando la capacità\n");
                        ptr_shm_port[*id_porto].g_received++;
                        merce_scambiata = merce_scambiata + stiva[z].size;
                    }
                    printf("la domanda per questa merce ora è: %i\n", domanda_porto[z].lotti);
                }
            }
        }
        tmp_load = merce_scambiata / ptr_shm_v_conf->so_loadspeed;
        nano_load->tv_sec = (int)tmp_load;
        nano_load->tv_nsec = (long int)(tmp_load - (int)tmp_load);
        nanosleep(nano_load, NULL);
    }
    tmp_load = merce_scambiata / ptr_shm_v_conf->so_loadspeed;
    nano_load->tv_sec = (int)tmp_load;
    nano_load->tv_nsec = (long int)(tmp_load - (int)tmp_load);
    nanosleep(nano_load, NULL);

    shmdt(offerta_porto);
    shmdt(domanda_porto);
    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    semop(ptr_shm_sem[1], &sops, 1); /*aggiungo al semaforo della memoria condivisa*/
    printf("ho terminato di interagire con il porto, ora rilascio la risorsa della memoria condivisa\n");
}
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