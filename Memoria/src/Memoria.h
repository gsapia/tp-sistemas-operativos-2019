#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

/*
#define SELECT "SELECT"
#define INSERT "INSERT"
#define CREATE "CREATE"
#define DESCRIBE "DESCRIBE"
#define DROP "DROP"
#define JOURNAL "JOURNAL"*/

struct Config{
	int puerto_escucha;
	char* ip_fs;
	int puerto_fs;
	char** ip_seeds;
	int* puertos_seeds;
	int retardo_acc_mp;
	int retardo_acc_fs;
	int tamanio_memoria;
	int tiempo_journal;
	int tiempo_gossiping;
	int numero_memoria;
}config;

t_log* logger;

struct pagina{
	long int timestamp; // Revisar bien el tipo
	u_int16_t key;
	char* valor; // El valor tiene un tamanio maximo informado por LFS desde el comienzo.
};

#endif /* MEMORIA_H_ */
