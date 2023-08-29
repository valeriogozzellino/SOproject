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
 * create_goods: This function generates type of good for offers and demands for port in a simulated trading scenario.
 * It calculates random IDs for the offered and demanded goods based on the available goods.
 * The function distributes these offers and demands across multiple days, maintaining consistent values for each good across all days.
 * It is essential for initializing the goods trading process with randomized variation across different days.
 */
void create_goods(struct var_conf *ptr_shm_var_conf, struct good *ptr_shm_good, struct good **domanda_days, struct good **offerta_days, int type_offered, int type_asked);
/**
 * create_lots: This function creates lots of goods for both offers and demands.
 * It takes tonnage availability into account and distributes goods into lots based on size and availability.
 * It ensures that goods are allocated to lots in a balanced manner, considering the available tonnage for both offers and demands.
 * It plays a key role in determining the distribution and quantities of goods for trading.
 */
void create_lots(struct good **domanda_days, struct good **offerta_days, int ton_days, int type_offered, int type_asked, int id_porto, int days_real);
/**
 * expired_good: This function manages expired goods within the trading system.
 * It iterates through the goods and their lots, and if a good has expired and still has available lots, it updates statistics for the expired goods and clears the lots.
 * This function is important for maintaining accurate records of expired goods and managing their impact on the trading simulation.
 */
void expired_good(struct good **offerta_days, struct good *ptr_shm_good, struct var_conf *ptr_shm_v_conf, int type_offered, int id_porto, int days_real);
/**
 * check_good: This function checks the status of remaining goods and lots at a given port over the course of the simulation.
 * It prints information about goods that have not been traded, both for offers and demands.
 * Additionally, it displays the total offered and demanded goods at the specified port.
 */
void check_good(struct good **domanda_days, struct good **offerta_days, struct var_conf *ptr_shm_v_conf, struct port *ptr_shm_porto, int ton_days, int type_offered, int type_asked, int id_porto);
/**
 * receive_message: This function receives and processes messages related to ship attacks on a port. ù
 * It retrieves messages from a message queue and displays information about the attack event.
 */
void receive_message(int id_msg);
#endif /*__HEADERPORT_H */