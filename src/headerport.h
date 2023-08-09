#ifndef __HEADERPORT_H
#define __HEADERPORT_H

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
/*----PROTOTIPI DI FUNZIONI-----*/
void create_goods(struct var_conf *ptr_shm_v_conf, struct good *ptr_shm_good, struct good **domanda_days, struct good **offerta_days, int id_shm_domanda, int id_shm_offerta, int type_offered, int type_asked, int id_porto);
void create_lots(struct good **domanda_days, struct good **offerta_days, int ton_days, int type_offered, int type_asked, int id_porto, int days_real);

#endif /*__HEADERPORT_H */