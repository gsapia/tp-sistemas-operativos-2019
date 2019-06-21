#include "Memoria.h"
#include "Config.h"
#include "IPC.h"
#include "Consola.h"
#include "MemoriaPrincipal.h"
#include "Gossiping.h"

int main(void) {
	logger = log_create("Memoria.log", "Memoria", 1, LOG_LEVEL_TRACE);
	log_info(logger, "Iniciando proceso Memoria");

	initCofig();

	// 1. Conectarse al proceso File System y realizar handshake necesario para obtener los datos requeridos. Esto incluye el tamaño máximo del Value configurado para la administración de las páginas.
	pthread_t hiloCliente;
	if (pthread_create(&hiloCliente, NULL, (void*)initCliente, NULL)) {
		log_error(logger, "Hilo cliente: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}

	pthread_join(hiloCliente, NULL); // Hasta que no este el handhake con FS hecho no deberiamos poder hacer mas nada

	// ----- Provisoriamente supongo un valor de longitud 4 bytes. Pero habria que usar el valor enviado por LFS -----
	//tamanio_value = 15;
	// ----- Fin parte provisoria -----

	pthread_t hiloServidor;
	if (pthread_create(&hiloServidor, NULL, (void*)servidor, NULL)) {
		log_error(logger, "Hilo servidor: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}

	// 2. Inicializar la memoria principal (que se explican en los siguientes apartados).
	initMemoriaPrincipal();

	// 3. Iniciar el proceso de Gossiping (explicado en profundidad en el Anexo III) que consiste en la comunicación de cada proceso memoria con otros procesos memorias, o seeds, para intercambiar y descubrir otros procesos memorias que se encuentren dentro del pool (conjunto de memorias).
	pthread_t hiloGossiping;
	if (pthread_create(&hiloGossiping, NULL, (void*)gossiping, NULL)) {
		log_error(logger, "Hilo gossiping: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}
	pthread_detach(hiloGossiping);

	// Ahora que iniciamos el resto, levantamos la consola y ejecutamos hasta que termine
	pthread_t hiloConsola;
	if (pthread_create(&hiloConsola, NULL, (void*)consola, NULL)) {
		log_error(logger, "Hilo consola: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}
	pthread_join(hiloConsola, NULL);

	//En caso que no pueda realizar alguna de las dos primeras operaciones, se deberá abortar el proceso memoria informando cuál fue el problema.

	// Salimos limpiamente
	pthread_cancel(hiloServidor); // Manda senial a hiloServidor de terminarse
	pthread_join(hiloServidor, NULL); // Esperamos a que hiloServidor termine
	closeCliente(); // Nos desconectamos de FS
	log_destroy(logger); // Liberamos memoria del log

	return EXIT_SUCCESS;
}
