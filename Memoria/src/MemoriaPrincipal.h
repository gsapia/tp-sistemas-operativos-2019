#ifndef MEMORIAPRINCIPAL_H_
#define MEMORIAPRINCIPAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <pthread.h>

#include "Memoria.h"

#define CANT_PAGINAS ( config.tamanio_memoria / tamanio_pagina )

typedef void* t_marco;

void* memoria_principal;

typedef struct{
	uint64_t timestamp;
	u_int16_t key;
	char* valor; // El valor tiene un tamanio maximo informado por LFS desde el comienzo.
}t_registro;

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
	void* marco;
	bool modificado;
}t_pagina;

t_list* tabla_segmentos;

void initMemoriaPrincipal();

pthread_mutex_t mutex_memoria_principal;

// Busca el segmento correspondiente a la tabla, sino encuentra ninguno, retorna NULL
t_segmento* buscar_segmento(char* nombre_tabla);

// Busca la pagina correspondiente a la key dada, sino encuentra ninguna, retorna NULL
t_pagina* buscar_pagina(t_segmento* segmento, uint16_t clave);

t_registro leer_registro(t_pagina* pagina);
void modificar_registro(t_pagina* pagina, char* valor);

t_pagina* inicializar_nuevo_registro(char* nombreTabla, u_int16_t key, char* valor);

t_pagina* agregar_registro(uint16_t clave, char* valor, t_segmento* segmento);

t_segmento* agregar_segmento(char* nombreTabla);

t_registro* buscar_registro(char* nombre_tabla, uint16_t key);


void vaciar_memoria();

#endif /* MEMORIAPRINCIPAL_H_ */
