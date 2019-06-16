#ifndef MISC_H_
#define MISC_H_

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

/**
 * @NAME: getTimestamp
 * @DESC: Devuelve el timestamp actual en milisegundos
 */
uint64_t getTimestamp();

/**
 * @NAME: msleep
 * @DESC: Duerme n milisegundos
 */
int msleep(int milisegundos);

#endif /* MISC_H_ */
