#ifndef LFS_H_
#define LFS_H_

#include<commons/log.h>
#include<commons/config.h>
#include<inttypes.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<readline/readline.h>
#include<pthread.h>
#include<unistd.h>
#include<string.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<signal.h>
#include<sys/socket.h>
#include<netdb.h>

#define SELECT "SELECT"
#define INSERT "INSERT"
#define CREATE "CREATE"
#define DESCRIBE "DESCRIBE"
#define DROP "DROP"
#define EXIT "EXIT"
#define IP "127.0.0.1"

t_log* logger;

typedef struct{
	double timeStamp;
	uint16_t key;
	char* value;
	struct Registro *siguiente;
}Registro;


// ##### SOCKETS #####
typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

#endif /* LFS_H_ */
