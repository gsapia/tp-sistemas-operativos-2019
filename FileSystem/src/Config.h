#ifndef CONFIGF_H_
#define CONFIGF_H_

#include <stdlib.h>
#include <sys/inotify.h>
#include <commons/config.h>
#include "Shared.h"

t_config* configf;

struct Config{
	int puerto_escucha;
	char* puntoMontaje;
	int retardo;
	int tamanio_value;
	int tiempo_dump;
}config;

void initCofig();
void leerConfig();


#endif /* CONFIG_H_ */
