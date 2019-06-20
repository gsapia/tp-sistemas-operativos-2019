#ifndef MEMORIAS_H_
#define MEMORIAS_H_

#include "serializacion.h"
#include "Kernel.h"
#include <commons/collections/list.h>

t_list* pool_memorias;

t_list* listasMemorias[3];

// Funcion que cada cierto tiempo pide la tabla de gossip a memoria
void gossip();

t_memoria* getMemoria(int numero);

#endif /* MEMORIAS_H_ */
