#include "Config.h"
#include <unistd.h>
#include <readline/readline.h>

#define CONFIG_PATH "LFS.config"

void leerConfig(){
	configf = config_create(CONFIG_PATH);
	if(!configf){
		log_error(logger, "ERROR: No se encuentra el archivo de configuracion.");
		exit(EXIT_FAILURE);
	}
	config.puerto_escucha = config_get_int_value(configf, "PUERTOESCUCHA");
	config.puntoMontaje = strdup(config_get_string_value(configf, "PUNTOMONTAJE"));
	config.retardo = config_get_int_value(configf, "RETARDO");
	config.tamaño_value = config_get_int_value(configf, "TAMAÑOVALUE");
	config.tiempo_dump = config_get_int_value(configf, "TIEMPODUMP");
	config_destroy(configf);
}

void actualizar_config(){
	int fd = inotify_init();
	inotify_add_watch(fd, CONFIG_PATH, IN_MODIFY);
	size_t tam = sizeof(struct inotify_event) + strlen(CONFIG_PATH) + 1;
	char buffer[tam];

	while(read(fd, buffer, tam)){
		config.puerto_escucha = config_get_int_value(configf, "PUERTOESCUCHA");
		config.puntoMontaje = strdup(config_get_string_value(configf, "PUNTOMONTAJE"));
		config.retardo = config_get_int_value(configf, "RETARDO");
		config.tamaño_value = config_get_int_value(configf, "TAMAÑOVALUE");
		config.tiempo_dump = config_get_int_value(configf, "TIEMPODUMP");
		config_destroy(configf);

		log_trace(logger, "Archivo de configuracion modificado, nuevos valores: PUERTOESCUCHA = %d, PUNTOMONTAJE = %s, RETARDO = %d, TAMAÑOVALUE = %d, TIEMPODUMP = %d",
				config.puerto_escucha, config.puntoMontaje, config.retardo, config.tamaño_value, config.tiempo_dump);
	}
}

void initCofig(){
	leerConfig();

	pthread_t hiloConfig;
	if (pthread_create(&hiloConfig, NULL, (void*)actualizar_config, NULL)) {
		log_error(logger, "Hilo config: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}
	pthread_detach(hiloConfig);
}
