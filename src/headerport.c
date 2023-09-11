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
    srand(time(NULL) ^ getpid());
    for (j = 0; j < type_offered; j++)
    {
        random_id = rand() % ptr_shm_var_conf->so_merci;
        offerta_days[0][j].id = random_id;
        offerta_days[0][j].size = ptr_shm_good[random_id].size;
        offerta_days[0][j].life = ptr_shm_good[random_id].life;
    }

    for (i = 1; i < ptr_shm_var_conf->so_days; i++)
    {
        for (j = 0; j < type_offered; j++)
        {
            offerta_days[i][j].id = offerta_days[0][j].id;
            offerta_days[i][j].size = offerta_days[0][j].size;
            offerta_days[i][j].life = ptr_shm_good[offerta_days[0][j].id].life;
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

void create_lots(struct good **domanda_days, struct good **offerta_days, int ton_days, int type_offered, int type_asked, int id_porto, int days)
{
    double ton_disp = ton_days / 2;
    double ton_disp2 = ton_days / 2;
    double ton_disp3;

    /*creation of lots for offerta and domanda*/
    int creazione = 1;
    int j;
    while (creazione != 0)
    {
        creazione = 0;

        for (j = 0; j < type_offered; j++)
        {
            if (offerta_days[days][j].size < ton_disp)
            {
                creazione = 1;
                offerta_days[days][j].lotti++;
                offerta_days[days][j].life += days;
                ton_disp -= offerta_days[days][j].size;
            }
        }
    }
    creazione = 1;
    while (creazione != 0)
    {
        creazione = 0;
        for (j = 0; j < type_asked; j++)
        {
            if (domanda_days[days][j].size < ton_disp2)
            {
                creazione = 1;
                domanda_days[days][j].lotti++;
                ton_disp2 -= domanda_days[days][j].size;
            }
        }
    }

    ton_disp3 = ton_disp2 + ton_disp;

    for (j = 0; j < type_offered; j++)
    {
        if (offerta_days[days][j].size < ton_disp3)
        {
            ton_disp3 -= offerta_days[days][j].size;
            offerta_days[days][j].lotti++;
            offerta_days[days][j].life += days;
        }
    }

    for (j = 0; j < type_asked; j++)
    {
        if (domanda_days[days][j].size < ton_disp2 && domanda_days[days][j].size < ton_disp3)
        {
            ton_disp3 -= domanda_days[days][j].size;
            domanda_days[days][j].lotti++;
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
                        ptr_shm_good[z].recap.port_expired += offerta_days[i][j].lotti;
                        offerta_days[i][j].lotti = 0;
                    }
                }
            }
        }
    }
}
void check_good(struct good **domanda_days, struct good **offerta_days, struct var_conf *ptr_shm_v_conf, struct port *ptr_shm_porto, int ton_days, int type_offered, int type_asked, int id_porto)
{
    int i, j;
    for (i = 0; i < ptr_shm_v_conf->so_days; i++)
    {

        for (j = 0; j < type_offered; j++)
        {
            if (offerta_days[i][j].lotti > 0)
            {
                printf("DUMP goods offered remained in the day %i: Porto [%i] -->  merci [%i] --> [%i] lotti \n", i, id_porto, offerta_days[i][j].id, offerta_days[i][j].lotti);
            }
        }
    }
    for (i = 0; i < ptr_shm_v_conf->so_days; i++)
    {
        for (j = 0; j < type_asked; j++)
        {
            if (domanda_days[i][j].lotti > 0)
            {
                printf("DUMP goods asked remained in the day %i: Porto [%i] -->  merce [%i] --> [%i] lotti \n", i, id_porto, domanda_days[0][j].id, domanda_days[0][j].lotti);
            }
        }
    }
    printf("PORT %i: merce totale offerta: %i\n", id_porto, ptr_shm_porto[id_porto].g_send);
    printf("PORT %i: merce totale domandata: %i\n", id_porto, ptr_shm_porto[id_porto].g_received);
}
void receive_message(int id_msg)
{
    struct Message msg;
    msg.messageType = 1;
    if (msgrcv(id_msg, &msg, sizeof(msg.messageText), msg.messageType, IPC_NOWAIT) == -1)
    {
    }
    else
    {
        printf("PORT: ship attack the port (%s)\n", msg.messageText);
    }
}