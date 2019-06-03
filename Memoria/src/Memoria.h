#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

struct Config{
	int puerto_escucha;
	char* ip_fs;
	int puerto_fs;
	char** ip_seeds;
	uint16_t* puertos_seeds;
	int retardo_acc_mp;
	int retardo_acc_fs;
	int tamanio_memoria;
	int tiempo_journal;
	int tiempo_gossiping;
	int numero_memoria;
}config;

t_log* logger;

#endif /* MEMORIA_H_ */
