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

/**
 * ship_move_first_position: This function moves a ship to the nearest port's position at the start.
 * It calculates distances between the ship and all available ports,then selects the closest port.
 * It uses nanosleep to simulate the travel time and updates the ship's position.
 * It's useful for initializing the ship's position upon application launch.
 */
void ship_move_first_position(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int *id_porto, int id_ship);
/**
 * ship_move_to: This function moves a ship to the next port.
 * If the current port is the last one in the array of distance about the port in the position (0,0),the ship is moved to the first port.
 * It uses nanosleep to simulate the travel time based on the ship's speed.
 * The ship's position is updated to the new port's position. It's useful for advancing a ship along the predefined route between ports.
 */
void ship_move_to(struct ship *ptr_shm_ship, struct port *ptr_shm_port, struct var_conf *ptr_shm_v_conf, int *id_porto, int id_ship);
/**
 * ship_expired_good: This function manages expired goods on a ship. It iterates through all goods on the ship, and if a good has expired on any of the simulation
 * days and still has available lots,
 * it updates the good's statistics and resets the lot count to zero. It's useful for monitoring and handling expired goods on the ship during the simulation.
 */
void ship_expired_good(struct ship *ptr_shm_ship, struct var_conf *ptr_shm_v_conf, struct good *ptr_shm_good, int id_ship, struct good **stiva);

/**
 * sendAttackMessage: This function sends an attack message to the message queue associated with a port's dock.
 * It prepares a message with a specific type and text and sends it to the message queue using msgsnd.
 * It's useful for sending attack notifications or similar events to a port within the system.
 */
void sendAttackMessage(int portMessageQueue, const char *message);
#endif /*__HEADERSHIP_H */
