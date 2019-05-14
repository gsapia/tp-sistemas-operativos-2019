#include "MemoriaPrincipal.h"

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
}

t_marco getPagina(){
	// TODO
	return 0;
}
