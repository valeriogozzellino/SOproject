#ifndef __HEADERSHIP_H
#define __HEADERSHIP_H

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
#include "math.h"
#include "configuration.h"
#define MAX_MESSAGE_SIZE 100

/* Structure for the message data */
struct Message
{
    long messageType;
    char messageText[MAX_MESSAGE_SIZE];
};
/**
 * funzione di posizionaento nel primo porto della mappa, dopo la creazione della nave , quest'ultima vine posizionata nella mappa in modo
 * randomico, grazie a questa funzione la nave raggiungerà il porto più vicino
 */
void ship_move_first_position(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int *id_porto, int id_ship);
/**
 * funzione di movimento della nave, la nave si sposta lungo un tragitto prestabilito dalla distanza dei porti dal centro
 */
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int *id_porto, int id_ship);
/**
 * funzione richimaata nel momento in cui una nae arriva in porto e si trova sulla banchina, la nave verifica quali merce sono scadute nel viaggio
 * e lo notifica
 */
void ship_expired_good(struct ship *ptr_shm_ship, struct var_conf *ptr_shm_v_conf, struct good *ptr_shm_good, int id_ship, struct good **stiva);

/**
 * funzione che avverte un porto di essere attaccato alla sua banchina
 */
void sendAttackMessage(int portMessageQueue, const char *message);
#endif /*__HEADERSHIP_H */
