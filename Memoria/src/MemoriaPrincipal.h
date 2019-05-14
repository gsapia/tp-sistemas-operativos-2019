#ifndef MEMORIAPRINCIPAL_H_
#define MEMORIAPRINCIPAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <commons/collections/list.h>

#include "Memoria.h"

#define CANT_PAGINAS ( config.tamanio_memoria / tamanio_pagina )

typedef uint16_t t_marco;

void* memoria_principal;

/*struct registro{
	long int timestamp; // Revisar bien el tipo
	u_int16_t key;
	char valor[tam]; // El valor tiene un tamanio maximo informado por LFS desde el comienzo.
};*/
/*
 * Un registro va a tener lo siguiente:
 * uint64_t timestamp;
 * uint16_t key;
 * char valor[tamanio_value]; // El valor tiene un tamanio maximo informado por LFS desde el comienzo.
 */
// Entonces:
#define DESPL_TIMESTAMP 0 // posicion inicial
#define DESPL_KEY (DESPL_TIMESTAMP+8) // 8 bytes del timestamp
#define DESPL_VALOR (DESPL_KEY+2) // 2 bytes de la key


uint16_t tamanio_value; // Tamanio del valor de un registro expresado en bytes
uint16_t tamanio_pagina; // tamanio de un registro expresado en bytes

typedef struct{
	char* nombre_tabla;
	t_list* paginas;
}t_segmento;

typedef struct{
	uint numero;
	t_marco marco;
	bool modificado;
}t_pagina;

t_list* tabla_segmentos;

void initMemoriaPrincipal();

#endif /* MEMORIAPRINCIPAL_H_ */
