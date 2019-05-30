#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

enum operaciones{
	SELECT = 1,
	INSERT = 2,
	CREATE = 3,
	DESCRIBE = 4,
	DROP = 5,
	JOURNAL
};

enum consistencias{
	SC = 1,
	SHC,
	EC
};

enum id_proceso{
	ID_KERNEL = 1,
	ID_MEMORIA,
	ID_FILESYSTEM
};

typedef struct{
	char* nombreTabla;
	uint16_t key;
}struct_select;

typedef struct{
	char* nombreTabla;
	uint16_t key;
	char* valor;
	uint64_t timestamp;
}struct_insert, struct_registro;

typedef struct{
	char* nombreTabla;
	uint8_t consistencia;
	uint16_t particiones;
	uint32_t tiempoCompactacion;
}struct_create;

typedef struct{
	char* nombreTabla;
}struct_describe, struct_drop;

struct_select recibir_select(int socket);

/**
 * @DESC: Recibe un INSERT sin timestamp
 */
struct_insert recibir_insert(int socket);

/**
 * @DESC: Recibe un INSERT con timestamp
 */
struct_insert recibir_insert_ts(int socket);

struct_create recibir_create(int socket);

struct_describe recibir_describe(int socket);

struct_describe recibir_drop(int socket);

void enviar_select(int socket, struct_select paquete);

/**
 * @NAME: enviar_insert
 * @DESC: Envia un INSERT sin timestamp
 */
void enviar_insert(int socket, struct_insert paquete);

/**
 * @DESC: Envia un INSERT con timestamp
 */
void enviar_insert_ts(int socket, struct_insert paquete);

void enviar_create(int socket, struct_create paquete);

void enviar_describe(int socket, struct_describe paquete);

void enviar_drop(int socket, struct_describe paquete);

void enviar_journal(int socket);


struct_registro recibir_registro(int socket);

void enviar_registro(int socket, struct_registro registro);


#endif /* SERIALIZACION_H_ */
