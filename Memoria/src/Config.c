#include "Config.h"
#include <unistd.h>
#include <readline/readline.h>

#define CONFIG_PATH "Memoria.config"

void leerConfig(){
	configf = config_create(CONFIG_PATH);
	if(!configf){
		log_error(logger, "ERROR: No se encuentra el archivo de configuracion.");
		exit(EXIT_FAILURE);
	}
	config.puerto_escucha = config_get_int_value(configf, "PUERTO");
	config.ip_fs = strdup(config_get_string_value(configf, "IP_FS"));
	config.puerto_fs = config_get_int_value(configf, "PUERTO_FS");
	config.ip_seeds = config_get_array_value(configf, "IP_SEEDS");

	char** str_puertos_seeds = config_get_array_value(configf, "PUERTO_SEEDS");

	int cantSeeds = 0;
	for(cantSeeds = 0; str_puertos_seeds[cantSeeds]; cantSeeds++); // Cuento la cantidad de puertos en el array

	config.puertos_seeds = malloc(sizeof(int) * cantSeeds); // El array de puertos va a tener tamanio igual a la cantidad de seeds.
															// O sea, uno menos que el array de IPs (ya que este ultimo tiene un ultimo lugar para el NULL)
															// La idea es que cuando se necesiten, se accedan a IP y Puerto en simultaneo, asi que controlariamos con el array de IPs

	for(int i = 0; i < cantSeeds; i++){
		config.puertos_seeds[i] = strtol(str_puertos_seeds[i], NULL, 10);
		free(str_puertos_seeds[i]);
	}
	free(str_puertos_seeds);

	config.retardo_acc_mp = config_get_int_value(configf, "RETARDO_MEM");
	config.retardo_acc_fs = config_get_int_value(configf, "RETARDO_FS");
	config.tamanio_memoria = config_get_int_value(configf, "TAM_MEM");
	config.tiempo_journal = config_get_int_value(configf, "RETARDO_JOURNAL");
	config.tiempo_gossiping = config_get_int_value(configf, "RETARDO_GOSSIPING");
	config.numero_memoria = config_get_int_value(configf, "MEMORY_NUMBER");
	config_destroy(configf);
}

void actualizar_config(){
	int fd = inotify_init();
	inotify_add_watch(fd, CONFIG_PATH, IN_MODIFY);
	size_t tam = sizeof(struct inotify_event) + strlen(CONFIG_PATH) + 1;
	char buffer[tam];

	while(read(fd, buffer, tam)){
		configf = config_create(CONFIG_PATH);
		config.retardo_acc_mp = config_get_int_value(configf, "RETARDO_MEM");
		config.retardo_acc_fs = config_get_int_value(configf, "RETARDO_FS");
		config.tiempo_journal = config_get_int_value(configf, "RETARDO_JOURNAL");
		config.tiempo_gossiping = config_get_int_value(configf, "RETARDO_GOSSIPING");
		config_destroy(configf);

		log_trace(logger, "Archivo de configuracion modificado, nuevos valores: RETARDO_MEM = %d, RETARDO_FS = %d, RETARDO_JOURNAL = %d, RETARDO_GOSSIPING = %d",
				config.retardo_acc_mp, config.retardo_acc_fs, config.tiempo_journal, config.tiempo_gossiping);
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
