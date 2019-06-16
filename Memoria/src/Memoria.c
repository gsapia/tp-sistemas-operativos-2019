#include "Memoria.h"
#include "API.h"
#include "IPC.h"
#include "Consola.h"
#include "MemoriaPrincipal.h"
#include <sys/inotify.h>

#define CONFIG_PATH "Memoria.config"

void leerConfig();
void initMemoriaPrincipal();

t_config* configf;
void initCofig();

int main(void) {
	logger = log_create("Memoria.log", "Memoria", 1, LOG_LEVEL_TRACE);
	log_info(logger, "Hola Soy Memoria");

	initCofig();

	pthread_t hiloCliente;
	if (pthread_create(&hiloCliente, NULL, (void*)initCliente, NULL)) {
		log_error(logger, "Hilo cliente: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}

	pthread_t hiloServidor;
	if (pthread_create(&hiloServidor, NULL, (void*)servidor, NULL)) {
		log_error(logger, "Hilo servidor: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}

	pthread_join(hiloCliente, NULL); // Hasta que no este el handhake con FS hecho no deberiamos poder hacer mas nada

	// ----- Provisoriamente supongo un valor de longitud 4 bytes. Pero habria que usar el valor enviado por LFS -----
	//tamanio_value = 15;
	// ----- Fin parte provisoria -----

	initMemoriaPrincipal();

	pthread_t hiloConsola;
	if (pthread_create(&hiloConsola, NULL, (void*)consola, NULL)) {
		log_error(logger, "Hilo consola: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}
	pthread_join(hiloConsola, NULL);



	// 1. Conectarse al proceso File System y realizar handshake necesario para obtener los datos requeridos. Esto incluye el tamaño máximo del Value configurado para la administración de las páginas.

	// 2. Inicializar la memoria principal (que se explican en los siguientes apartados).

	// 3. Iniciar el proceso de Gossiping (explicado en profundidad en el Anexo III) que consiste en la comunicación de cada proceso memoria con otros procesos memorias, o seeds, para intercambiar y descubrir otros procesos memorias que se encuentren dentro del pool (conjunto de memorias).

	//En caso que no pueda realizar alguna de las dos primeras operaciones, se deberá abortar el proceso memoria informando cuál fue el problema.

	// Salimos limpiamente
	pthread_cancel(hiloServidor); // Manda senial a hiloServidor de terminarse
	pthread_join(hiloServidor, NULL); // Esperamos a que hiloServidor termine
	log_destroy(logger); // Liberamos memoria del log
	config_destroy(configf); // Liberamos memoria de archivo de config

	return EXIT_SUCCESS;
}

void leerConfig(){
	configf = config_create(CONFIG_PATH);
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
		log_trace(logger, "Lei puerto de seeds: %d", config.puertos_seeds[i]); // Esto nomas para probar, pero esta de mas y habria que sacarlo
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
