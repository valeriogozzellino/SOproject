/*in questo file andò ad inizializzare tutte le variabili e le strutture utili nel progetto ad esempio:
le variabili contenute nella shared memory(merce e variabili di configurazione, variaili che utilizzo per
 passare angometnti nella exec e vanno convertiti in char)*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <math.h>
#include "configuration.h"

/*----match of the value in file_configurazione.txt-----*/
void find_val(struct var_conf *env_var)
{
    FILE *f_c;
    int i = 0, j = 0, cnt = 0;
    char *number, c;

    number = malloc(sizeof(char) * 100);

    open_file(&f_c);

    while ((c = fgetc(f_c)) != EOF)
    {
        if (c == '=')
        {
            j = 0;
            while ((c = fgetc(f_c)) != '\n')
            {
                number[j] = c;
                j++;
            }
            cnt++;

            switch (cnt)
            {
            case 1:
                env_var->so_days = atoi(number);
                fprintf(stderr, "SO_DAYS: %i \n", env_var->so_days);
                break;
            case 2:
                env_var->so_navi = atoi(number);
                printf("SO_NAVI: %i \n", env_var->so_navi);
                break;
            case 3:
                env_var->so_porti = atoi(number);
                printf("SO_PORTI: %i \n", env_var->so_porti);
                break;
            case 4:
                env_var->so_merci = atoi(number);
                printf("SO_MERCI: %i \n", env_var->so_merci);
                break;
            case 5:
                env_var->so_size = atoi(number);
                printf("SO_SIZE: %i \n", env_var->so_size);
                break;
            case 6:
                env_var->so_min_vita = atoi(number);
                printf("SO_MIN_VITA: %i \n", env_var->so_min_vita);
                break;
            case 7:
                env_var->so_max_vita = atoi(number);
                printf("SO_MAX_VITA: %i \n", env_var->so_max_vita);
                break;
            case 8:
                env_var->so_lato = atof(number);
                printf("SO_LATO: %f \n", env_var->so_lato);
                break;

            case 9:
                env_var->so_speed = atoi(number);
                printf("SO_SPEED: %i \n", env_var->so_speed);
                break;
            case 10:
                env_var->so_banchine = atoi(number);
                printf("SO_BANCHINE: %i \n", env_var->so_banchine);
                break;

            case 11:
                env_var->so_loadspeed = atoi(number);
                printf("SO_LOADSPEED: %i \n", env_var->so_loadspeed);
                break;
            case 12:
                env_var->so_fill = atoi(number);
                printf("SO_FILL: %i \n", env_var->so_fill);
                break;
            case 13:
                env_var->so_capacity = atoi(number);
                printf("SO_CAPACITY: %i \n", env_var->so_capacity);
                break;
            }
        }
    }

    free(number);
    fclose(f_c);
}

/*----open file_configurazione to read the SO_variables---*/
void open_file(FILE **f_c)
{
    if ((*f_c = fopen("./file_configurazione.txt", "r")) == NULL)
    {
        printf("error in fopen in file: %s \n", __FILE__);
    }
}

/*----inizializzo valori sh memori per parametri di configurazione-----*/
void sh_memory_v_conf(struct var_conf env_var, struct var_conf *ptr_shm_v_conf)
{
    ptr_shm_v_conf->so_days = env_var.so_days;
    ptr_shm_v_conf->so_navi = env_var.so_navi;
    ptr_shm_v_conf->so_porti = env_var.so_porti;
    ptr_shm_v_conf->so_merci = env_var.so_merci;
    ptr_shm_v_conf->so_size = env_var.so_size;
    ptr_shm_v_conf->so_speed = env_var.so_speed;
    ptr_shm_v_conf->so_min_vita = env_var.so_min_vita;
    ptr_shm_v_conf->so_max_vita = env_var.so_max_vita;
    ptr_shm_v_conf->so_lato = env_var.so_lato;
    ptr_shm_v_conf->so_fill = env_var.so_fill;
    ptr_shm_v_conf->so_banchine = env_var.so_banchine;
    ptr_shm_v_conf->so_loadspeed = env_var.so_loadspeed;
    ptr_shm_v_conf->so_capacity = env_var.so_capacity;
}
/*-----inizialization of good in sharem memory---*/
void sh_memory_v_good(struct var_conf env_var, struct good *ptr_shm_good)
{
    int i;
    for (i = 0; i < env_var.so_merci; i++)
    {
        ptr_shm_good[i].id = i;                                                                              // id del lotto
        ptr_shm_good[i].size = (rand() % env_var.so_size) + 1;                                               // peso del lotto
        ptr_shm_good[i].life = (rand() % (env_var.so_max_vita - env_var.so_min_vita)) + env_var.so_min_vita; // vita del lotto
        // printf("merce: id[%i],size[%i], life[%i]\n", ptr_shm_good[i].id, ptr_shm_good[i].size, ptr_shm_good[i].life);
    }
}
void set_good_ship(struct good *ptr_shm_good, struct good **stiva, struct var_conf env_var)
{
    for (int i = 0; i < env_var.so_days; i++)
    {
        for (int j = 0; j < env_var.so_merci; j++)
        {
            stiva[i][j].id = ptr_shm_good[j].id;
            stiva[i][j].life = ptr_shm_good[j].life;
            stiva[i][j].size = ptr_shm_good[j].size;
        }
    }
}

/*-----inizialization of port shared memory e assegnamento della loro posizione----*/
void sh_memory_v_porti(struct var_conf env_var, struct port *ptr_shm_port)
{
    int n_banchine = (rand() % env_var.so_banchine) + 1;
    for (int j = 0; j < env_var.so_porti; j++)
    {
        ptr_shm_port[j].n_banchine = n_banchine;
        ptr_shm_port[j].fill = env_var.so_fill;
    }
    // printf("Ogni porto ha : banchine [%i], fill [%i]", ptr_shm_port[0].n_banchine, ptr_shm_port[0].fill);
    //  rappresentazione asse cartesiano
    for (int i = 0; i < env_var.so_porti; i++)
    {
        switch (i)
        {
        case 0:
            ptr_shm_port[i].pos_porto.x = 0;
            ptr_shm_port[i].pos_porto.y = 0;
            break;
        case 1:
            ptr_shm_port[i].pos_porto.x = 0;
            ptr_shm_port[i].pos_porto.y = env_var.so_lato;
            break;
        case 2:
            ptr_shm_port[i].pos_porto.x = env_var.so_lato;
            ptr_shm_port[i].pos_porto.y = env_var.so_lato;
            break;
        case 3:
            ptr_shm_port[i].pos_porto.x = env_var.so_lato;
            ptr_shm_port[i].pos_porto.y = 0;
            break;
        default:
            ptr_shm_port[i].pos_porto.x = ((double)rand() / RAND_MAX * env_var.so_lato);
            ptr_shm_port[i].pos_porto.y = ((double)rand() / RAND_MAX * env_var.so_lato);
        }
    }
}

/*mi posiziona i porti in maniera ordinata per la distanza dal punto 0 nella  mappa in modo da stabilire un percorso per la nave se
semplicemente ciclando sull'array*/
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
    
}

/*-----inizialization of ship shared memory----*/
void sh_memory_v_ship(struct var_conf env_var, struct ship *ptr_shm_ship)
{
    for (int i = 0; i < env_var.so_navi; i++)
    {
        /*----capacità----*/
        ptr_shm_ship[i].capacity = env_var.so_capacity;
        /*-----velocità----*/
        ptr_shm_ship[i].speed = env_var.so_speed;
        /*---posizione----*/
        ptr_shm_ship[i].pos_ship.x = ((double)rand() / RAND_MAX * env_var.so_lato);
        ptr_shm_ship[i].pos_ship.y = ((double)rand() / RAND_MAX * env_var.so_lato);
        // printf("posizione_Ship[%i]^:(%f,%f), speed[%i],capacità[%i]\n", i, ptr_shm_ship[i].pos_ship.x, ptr_shm_ship[i].pos_ship.y, ptr_shm_ship[i].speed, ptr_shm_ship[i].capacity);
    }
}
void load_val_semaphor(int sem_id_banchine, int sem_id_shm, int sem_id, int *ptr_shm_semaphore, struct var_conf *ptr_shm_v_conf)
{
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        semctl(sem_id_banchine, i, SETVAL, ptr_shm_v_conf->so_banchine); // per accedere al porto, numero di banchine
    }
    for (int i = 0; i < ptr_shm_v_conf->so_porti; i++)
    {
        semctl(sem_id_shm, i, SETVAL, 1); // per accedere alle merci in ogni porto, uno per volta
    }
    semctl(sem_id, 0, SETVAL, 0); // start simulation
    ptr_shm_semaphore[0] = sem_id_banchine;
    ptr_shm_semaphore[1] = sem_id_shm;
    ptr_shm_semaphore[2] = sem_id;
}