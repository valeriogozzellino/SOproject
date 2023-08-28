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
/**
 * funzione che mi sceglie in modo randomico i tipi di merce che verranno offerti e quali domandati al porto
 */
void create_goods(struct var_conf *ptr_shm_var_conf, struct good *ptr_shm_good, struct good **domanda_days, struct good **offerta_days, int type_offered, int type_asked);
/**
 * funzione che distribuisce in modo equo le tonnellate giornaliere in modo da rispettare SO_FILL, si cerca
 * di ridurre al minimo gli sprechi soddisfando a pieno le tonnellate disponibili
 */
void create_lots(struct good **domanda_days, struct good **offerta_days, int ton_days, int type_offered, int type_asked, int id_porto, int days_real);
/**
 *funzione per il controllo della scadenza delle merci nl porto
 */
void expired_good(struct good **offerta_days, struct good *ptr_shm_good, struct var_conf *ptr_shm_v_conf, int type_offered, int id_porto, int days_real);
/*funzione di controllo merci per verifica dei lotti  nel dump di inizio e di fine simulazione*/
void check_good(struct good **domanda_days, struct good **offerta_days, struct var_conf *ptr_shm_v_conf, struct port *ptr_shm_porto, int ton_days, int type_offered, int type_asked, int id_porto);
void receive_message(int id_msg);
#endif /*__HEADERPORT_H */