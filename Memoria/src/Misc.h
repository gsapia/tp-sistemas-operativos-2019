#ifndef MISC_H_
#define MISC_H_

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "serializacion.h"

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

/**
 * @NAME: consistenciaAString
 * @DESC: Recibe una consistencia y devuelve un string para representarla
 */
char* consistenciaAString(enum consistencias consistencia);

#endif /* MISC_H_ */
