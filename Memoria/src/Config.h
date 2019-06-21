#ifndef CONFIGM_H_
#define CONFIGM_H_

#include <stdlib.h>
#include <commons/config.h>
#include <sys/inotify.h>
#include "Memoria.h"

t_config* configf;

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

void initCofig();
void leerConfig();

#endif /* CONFIGM_H_ */
