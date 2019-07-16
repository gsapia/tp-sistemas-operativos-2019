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

pthread_mutex_t mutex_metadata;

// Funcion que cada cierto tiempo pide la tabla de gossip a memoria
void gossip();

t_memoria* getMemoria(int numero);

enum consistencias obtener_consistencia(char* nombre_tabla);

void eliminar_memoria(t_memoria * memoria);


bool existeTabla(char* nombre_tabla);
t_memoria* obtener_memoria_random_del_pool();
t_memoria* obtener_memoria_segun_consistencia(enum consistencias consistencia, uint16_t key);
t_memoria* obtener_memoria_SC();
t_memoria* obtener_memoria_SHC(uint16_t key);
t_memoria* obtener_memoria_EC();


#endif /* MEMORIAS_H_ */
