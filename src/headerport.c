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

void create_goods(struct var_conf *ptr_shm_var_conf, struct good *ptr_shm_good, struct good **domanda_days, struct good **offerta_days, int type_offered, int type_asked)
{
    int j, i;
    int random_id;
    for (j = 0; j < type_offered; j++)
    {
        random_id = rand() % ptr_shm_var_conf->so_merci;
        offerta_days[0][j].id = random_id;
        offerta_days[0][j].size = ptr_shm_good[random_id].size;
        offerta_days[0][j].life = ptr_shm_good[random_id].life;
        /*printf("Tipi offerti---> merce id: (%i), size(%i), life(%i)\n", offerta_days[0][j].id, offerta_days[0][j].size, offerta_days[0][j].life);*/
    }

    for (i = 1; i < ptr_shm_var_conf->so_days; i++)
    {
        for (j = 0; j < type_offered; j++)
        {
            offerta_days[i][j].id = offerta_days[0][j].id;
            offerta_days[i][j].size = offerta_days[0][j].size;
            offerta_days[i][j].life = offerta_days[0][j].life + i;
        }
    }

    for (j = 0; j < type_asked; j++)
    {
        random_id = rand() % ptr_shm_var_conf->so_merci;
        domanda_days[0][j].id = random_id;
        domanda_days[0][j].size = ptr_shm_good[random_id].size;
    }

    for (i = 1; i < ptr_shm_var_conf->so_days; i++)
    {
        for (j = 0; j < type_asked; j++)
        {
            domanda_days[i][j].id = domanda_days[0][j].id;
            domanda_days[i][j].size = domanda_days[0][j].size;
        }
    }
}

void create_lots(struct good **domanda_days, struct good **offerta_days, int ton_days, int type_offered, int type_asked, int id_porto, int days_real)
{
    double ton_disp = ton_days / 2;
    double ton_disp2 = ton_days / 2;
    double ton_disp3;

    /*Creazione dei lotti per le merci offerte*/
    int creazione = 1;
    int j;
    while (creazione != 0)
    {
        creazione = 0;

        for (j = 0; j < type_offered; j++)
        {
            if (offerta_days[days_real][j].size < ton_disp)
            {
                creazione = 1;
                offerta_days[days_real][j].lotti++;

                offerta_days[days_real][j].life + days_real;
                ton_disp -= offerta_days[days_real][j].size;
            }
        }
    }
    creazione = 1;
    while (creazione != 0)
    {
        creazione = 0;
        for (j = 0; j < type_asked; j++)
        {
            if (domanda_days[days_real][j].size < ton_disp2)
            {
                creazione = 1;
                domanda_days[days_real][j].lotti++;
                ton_disp2 -= domanda_days[days_real][j].size;
            }
        }
    }

    ton_disp3 = ton_disp2 + ton_disp;

    for (j = 0; j < type_offered; j++)
    {
        if (offerta_days[days_real][j].size < ton_disp3)
        {
            ton_disp3 -= offerta_days[days_real][j].size;
            offerta_days[days_real][j].lotti++;
        }
    }

    for (j = 0; j < type_asked; j++)
    {
        if (domanda_days[days_real][j].size < ton_disp2 && domanda_days[days_real][j].size < ton_disp3)
        {
            ton_disp3 -= domanda_days[days_real][j].size;
            domanda_days[days_real][j].lotti++;
        }
    }
}
void expired_good(struct good **offerta_days, struct good *ptr_shm_good, struct var_conf *ptr_shm_v_conf, int type_offered, int id_porto, int days_real)
{
    int i, j, z;
    for (i = 0; i <= days_real; i++)
    {
        for (j = 0; j < type_offered; j++)
        {
            if (offerta_days[i][j].life <= days_real && offerta_days[i][j].lotti > 0)
            {
                for (z = 0; z < ptr_shm_v_conf->so_merci; z++)
                {
                    if (offerta_days[i][j].id == ptr_shm_good[z].id)
                    {
                        ptr_shm_good[j].recap.port_expired += offerta_days[i][j].lotti;
                    }
                }
                offerta_days[i][j].lotti = 0;
                printf("PORT %i: la merce %i è scaduta\n", id_porto, offerta_days[i][j].id);
            }
        }
    }
}