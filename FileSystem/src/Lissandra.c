#include "LFS.h"

int main(void){
	crearMutex();
	memTable = list_create(); cantDumps = 0;
	diccionario = dictionary_create();
	config = leer_config();
	puntoMontaje = config_get_string_value(config,"PUNTOMONTAJE");
	int tiempo_dump = config_get_int_value(config, "TIEMPODUMP");
	uint16_t puerto_escucha = config_get_int_value(config, "PUERTOESCUCHA");
	tamValue = config_get_int_value(config, "TAMAÑOVALUE");
	logger = iniciar_logger();
	log_info(logger, "Hola, soy Lissandra");

	argumentos_servidor args;
	args.puerto_escucha = puerto_escucha;
	args.tamValue = tamValue;

//	Servidor
	pthread_t hiloServidor;
	if(pthread_create(&hiloServidor, NULL, servidor, &args)){
		log_error(logger, "Hilo servidor: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}


	//Creo las Tablas del FileSystem necesarias
	pthread_t hiloFS;
	if(pthread_create(&hiloFS, NULL, fileSystem, NULL)){
		log_error(logger, "Hilo FileSystem: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}
	pthread_join(hiloFS,NULL);
	crearMutexTablasExistentes();
	crearCompactacionTablasExistentes();

	//Api Lissandra
	pthread_t hiloConsola;
	if(pthread_create(&hiloConsola, NULL, consola, NULL)){
		log_error(logger, "Hilo consola: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}

	pthread_t hiloDump;
	if(pthread_create(&hiloDump, NULL, dump, tiempo_dump)){
		log_error(logger, "Hilo Dump: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}

	pthread_join(hiloConsola, NULL);
//	pthread_join(hiloServidor,NULL);
	log_destroy(logger);

//	Libero todas las variables que me quedan colgadas.

	config_destroy(config);
	return EXIT_SUCCESS;
}

