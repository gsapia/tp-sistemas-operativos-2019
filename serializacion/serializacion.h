#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

enum operaciones{
	SELECT = 1,
	INSERT,
	CREATE,
	DESCRIBE,
	DESCRIBE_GLOBAL,
	DROP,
	JOURNAL,
	GOSSIP // Se usa por Kernel cuando quiere pedir la tabla de gossiping a memoria
};

enum consistencias{
	SC ,
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

enum estados_select{
	ESTADO_SELECT_OK,
	ESTADO_SELECT_ERROR_TABLA, // No existe la tabla
	ESTADO_SELECT_ERROR_KEY, // No existe esa key en la tabla
	ESTADO_SELECT_MEMORIA_FULL, // La memoria esta full
	ESTADO_SELECT_ERROR_OTRO
};

typedef struct{
	uint16_t estado;
	char* valor;
	uint64_t timestamp;
}struct_select_respuesta;

typedef struct{
	char* nombreTabla;
	uint16_t key;
	char* valor;
	uint64_t timestamp;
}struct_insert;

enum estados_insert{
	ESTADO_INSERT_OK,
	ESTADO_INSERT_ERROR_TABLA, // No existe la tabla
	ESTADO_INSERT_MEMORIA_FULL, // La memoria esta full
	ESTADO_INSERT_ERROR_OTRO
};

typedef struct{
	char* nombreTabla;
	uint8_t consistencia;
	uint16_t particiones;
	uint32_t tiempoCompactacion;
}struct_create;

enum estados_create{
	ESTADO_CREATE_OK,
	ESTADO_CREATE_ERROR_TABLAEXISTENTE, // La tabla a crear ya existia
	ESTADO_CREATE_ERROR_OTRO
};

typedef struct{
	char* nombreTabla;
}struct_describe, struct_drop;

enum estados_describe{
	ESTADO_DESCRIBE_OK,
	ESTADO_DESCRIBE_ERROR_TABLA, // No existe la tabla
	ESTADO_DESCRIBE_ERROR_OTRO
};

typedef struct{
	uint16_t estado;
	uint16_t consistencia;
	uint16_t particiones;
	uint32_t tiempo_compactacion;
}struct_describe_respuesta;

typedef struct{
	uint16_t estado;
	t_dictionary* describes; // Lista de struct_describe_respuesta para cada nombre de tabla
}struct_describe_global_respuesta;

enum estados_drop{
	ESTADO_DROP_OK,
	ESTADO_DROP_ERROR_TABLA, // No existe la tabla
	ESTADO_DROP_ERROR_OTRO
};

enum estados_journal{
	ESTADO_JOURNAL_OK,
	ESTADO_JOURNAL_ERROR_OTRO
};


typedef struct{
	uint32_t numero;
	char* IP;
	uint16_t puerto;
}t_memoria; // struct usado para el gossiping


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

struct_drop recibir_drop(int socket);

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

void enviar_drop(int socket, struct_drop paquete);

void enviar_journal(int socket);

void enviar_describe_global(int socket);


struct_select_respuesta recibir_registro(int socket);

void enviar_registro(int socket, struct_select_respuesta registro);


void responder_create(int socket, enum estados_create estado);

enum estados_create recibir_respuesta_create(int socket);

void responder_insert(int socket, enum estados_insert estado);

enum estados_insert recibir_respuesta_insert(int socket);

void enviar_respuesta_describe(int socket, struct_describe_respuesta respuesta);

struct_describe_respuesta recibir_respuesta_describe(int socket);

void enviar_respuesta_describe_global(int socket, struct_describe_global_respuesta respuesta);

struct_describe_global_respuesta recibir_respuesta_describe_global(int socket);

void responder_drop(int socket, enum estados_drop estado);

enum estados_drop recibir_respuesta_drop(int socket);

void responder_journal(int socket, enum estados_journal estado);

enum estados_journal recibir_respuesta_journal(int socket);

void enviar_tabla_gossiping(int socket, t_list* tabla);

t_list* recibir_tabla_gossiping(int socket);

#endif /* SERIALIZACION_H_ */
