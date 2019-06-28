#ifndef MEMORIAS_H_
#define MEMORIAS_H_

#include "serializacion.h"
#include "Kernel.h"
#include <commons/collections/list.h>

t_list* pool_memorias;

sem_t primer_gossip_hecho;

pthread_mutex_t mutex_pool_memorias;

t_list* listasMemorias[3];

typedef struct{
	enum consistencias consistencia;
	uint16_t particiones;
	uint32_t tiempo_compactacion;
}t_metadata;

t_dictionary* metadata; // Lista de metadata para cada nombre de tabla

// Funcion que cada cierto tiempo pide la tabla de gossip a memoria
void gossip();

t_memoria* getMemoria(int numero);

bool existeTabla(char* nombre_tabla);
t_memoria* obtener_memoria_random_del_pool();
t_memoria* obtener_memoria_segun_tabla(char* nombre_tabla, uint16_t key);
t_memoria* obtener_memoria_SC();
t_memoria* obtener_memoria_SHC(uint16_t key);
t_memoria* obtener_memoria_EC();


#endif /* MEMORIAS_H_ */
