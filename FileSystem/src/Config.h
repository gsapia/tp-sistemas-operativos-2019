#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdlib.h>
#include <sys/inotify.h>
#include <commons/config.h>

t_config* configf;

struct Config{
	int puerto_escucha;
	char* puntoMontaje;
	int retardo;
	int tamaño_value;
	int tiempo_dump;
}config;

void initCofig();
void leerConfig();


#endif /* CONFIG_H_ */
