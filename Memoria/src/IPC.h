#ifndef IPC_H_
#define IPC_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "Memoria.h"

enum operaciones{
	SELECT = 1,
	INSERT,
	CREATE,
	DESCRIBE,
	DROP,
	JOURNAL
};

typedef struct{
	uint16_t tamanio_nombre;
	char* nombreTabla;
	uint16_t key;
}struct_select;

typedef struct{
	uint16_t tamanio_nombre;
	char* nombreTabla;
	uint16_t key;
	uint16_t tamanio_valor;
	char* valor;
}struct_insert;

// Capaz es mejor no tener los tamanios en las structs y calcularos solo al enviar?
// No se, aca pruebo esta opcion, entonces la logica de las funciones de describe es un tanto distinta a las otras.
typedef struct{
	char* nombreTabla;
}struct_describe;

void cliente();
void servidor();

#endif /* IPC_H_ */
