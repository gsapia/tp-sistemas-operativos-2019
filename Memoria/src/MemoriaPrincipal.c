#include "MemoriaPrincipal.h"

bool* paginas_usadas; // Habria que modificar esto para usar LRU
pthread_mutex_t mutex_paginas;

void initMemoriaPrincipal(){
	log_trace(logger, "Inicializando la memoria principal con un tamanio de %d bytes", config.tamanio_memoria);
	memoria_principal = malloc(config.tamanio_memoria);
	if(memoria_principal == NULL){
		log_error(logger, "No se pudo reservar espacio para la memoria principal. Hay suficiente RAM disponible?");
		exit(EXIT_FAILURE);
	}

	// Inicializo el tamanio de pagina
	tamanio_pagina = sizeof(uint64_t) + sizeof(uint16_t) + tamanio_value;
	log_trace(logger, "Tendremos un total de %d paginas de un tamanio de %d bytes cada una", CANT_PAGINAS, tamanio_pagina);

	// Inicializo la tabla de segmentos
	tabla_segmentos = list_create();

	paginas_usadas = malloc(CANT_PAGINAS);
	for(uint32_t marco = 0; marco < CANT_PAGINAS; marco++)
		*(paginas_usadas + marco) = false;
}

t_marco getPagina(){
	// TODO: Habria que modificar esto para usar LRU

	pthread_mutex_lock(&mutex_paginas); // Mutex para evitar que dos hilos distintos tomen la misma pagina sin querer
	uint32_t marco;
	for(marco = 0; *(paginas_usadas + marco); marco++);

	*(paginas_usadas + marco) = true;

	pthread_mutex_unlock(&mutex_paginas);

	return memoria_principal + (tamanio_pagina * marco);
}
