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

void create_goods(struct var_conf *ptr_shm_var_conf, struct good *ptr_shm_good, struct good **domanda_days, struct good **offerta_days, int id_shm_domanda, int id_shm_offerta, int type_offered, int type_asked, int id_porto)
{
    printf("Ho %i tipi di merce offerta al porto %i\n", type_offered, id_porto);
    printf("Ho %i tipi di merce domandata al porto %i\n", type_asked, id_porto);

    // Creazione delle quantità dei tipi di merce offerti
    for (int i = 0; i < ptr_shm_var_conf->days_real; i++)
    {
        for (int j = 0; j < type_offered; j++)
        {
            offerta_days[i][j].id = rand() % ptr_shm_var_conf->so_merci;
            offerta_days[i][j].size = ptr_shm_good[offerta_days[i][j].id].size;
            offerta_days[i][j].life = ptr_shm_good[offerta_days[i][j].id].life;
            printf("Tipi offerti---> merce id: (%i), size(%i), life(%i)\n", offerta_days[i][j].id, offerta_days[i][j].size, offerta_days[i][j].life);
        }
    }

    // Creazione delle quantità dei tipi di merce domandati
    for (int i = 0; i < ptr_shm_var_conf->days_real; i++)
    {
        for (int j = 0; j < type_asked; j++)
        {

            domanda_days[i][j].id = rand() % ptr_shm_var_conf->so_merci;
            domanda_days[i][j].size = ptr_shm_good[domanda_days[i][j].id].size;
            printf("Tipi domandati----> merce id: (%i), size(%i)\n", domanda_days[i][j].id, domanda_days[i][j].size);
        }
    }
}

void create_lots(struct good **domanda_days, struct good **offerta_days, int ton_days, int type_offered, int type_asked, int id_porto, int days_real)
{
    double ton_disp = ton_days / 2;
    double ton_disp2 = ton_days / 2;
    double ton_disp3;

    // Creazione dei lotti per le merci offerte
    int creazione = 1;
    while (creazione != 0)
    {
        creazione = 0;
        for (int i = 0; i < days_real; i++)
        {

            for (int j = 0; j < type_offered; j++)
            {
                if (offerta_days[i][j].size < ton_disp)
                {
                    creazione = 1;
                    offerta_days[i][j].lotti++;
                    // aggiungere la life
                    ton_disp -= offerta_days[i][j].size;
                }
            }
        }
    }
    // Creazione dei lotti per le merci domandate
    creazione = 1;
    while (creazione != 0)
    {
        creazione = 0;
        for (int i = 0; i < days_real; i++)
        {

            for (int j = 0; j < type_asked; j++)
            {
                if (domanda_days[i][j].size < ton_disp2)
                {
                    creazione = 1;
                    domanda_days[i][j].lotti++;
                    ton_disp2 -= domanda_days[i][j].size;
                }
            }
        }
    }

    // Controllo per l'uso degli avanzi di tonnellaggio
    ton_disp3 = ton_disp2 + ton_disp;
    for (int i = 0; i < days_real; i++)
    {

        for (int j = 0; j < type_offered; j++)
        {
            if (offerta_days[i][j].size < ton_disp3)
            {
                ton_disp3 -= offerta_days[i][j].size;
                offerta_days[i][j].lotti++;
            }
        }
    }
    for (int i = 0; i < days_real; i++)
    {

        for (int j = 0; j < type_asked; j++)
        {
            if (domanda_days[i][j].size < ton_disp2 && domanda_days[i][j].size < ton_disp3)
            {
                ton_disp3 -= domanda_days[i][j].size;
                domanda_days[i][j].lotti++;
            }
        }
    }

    // Stampa dei risultati
    // for (int j = 0; j < type_asked; j++)
    // {
    //     printf("Porto [%i], merce-type [%i], lotti domandati (%i)\n", id_porto, domanda[j].id, domanda[j].lotti);
    // }
    // for (int j = 0; j < type_offered; j++)
    // {
    //     printf("Porto [%i], merce-type [%i], lotti in offerta creati (%i)\n", id_porto, offerta[j].id, offerta[j].lotti);
    // }
}