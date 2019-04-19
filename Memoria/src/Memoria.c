#include "Memoria.h"
#include "API.h"

void* consola();
void* servidor();
void* cliente();

struct Config{
	int puerto_escucha;
	char* ip_fs;
	int puerto_fs;
	char* ip_seeds[2];
	int puertos_seeds[2];
	int retardo_acc_mp;
	int retardo_acc_fs;
	int tamanio_memoria;
	int tiempo_journal;
	int tiempo_gossiping;
	int numero_memoria;
}config;
void leerConfig();

int main(void) {
	logger = log_create("Memoria.log", "Memoria", 1, LOG_LEVEL_TRACE);
	log_info(logger, "Hola Soy Memoria");

	leerConfig();

	pthread_t hiloConsola;
	if (pthread_create(&hiloConsola, NULL, consola, NULL)) {
		log_error(logger, "Hilo consola: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}

	pthread_t hiloServidor;
	if (pthread_create(&hiloServidor, NULL, servidor, NULL)) {
		log_error(logger, "Hilo servidor: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}

	pthread_t hiloCliente;
	if (pthread_create(&hiloCliente, NULL, cliente, NULL)) {
		log_error(logger, "Hilo cliente: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}
	pthread_join(hiloConsola, NULL);




	// 1. Conectarse al proceso File System y realizar handshake necesario para obtener los datos requeridos. Esto incluye el tamaño máximo del Value configurado para la administración de las páginas.

	// 2. Inicializar la memoria principal (que se explican en los siguientes apartados).

	// 3. Iniciar el proceso de Gossiping (explicado en profundidad en el Anexo III) que consiste en la comunicación de cada proceso memoria con otros procesos memorias, o seeds, para intercambiar y descubrir otros procesos memorias que se encuentren dentro del pool (conjunto de memorias).

	//En caso que no pueda realizar alguna de las dos primeras operaciones, se deberá abortar el proceso memoria informando cuál fue el problema.

	log_destroy(logger);

	return EXIT_SUCCESS;
}

void leerConfig(){
	t_config* configf = config_create("Memoria.config");
	config.puerto_escucha = config_get_int_value(configf, "PUERTO");
	log_trace(logger, "Lei puerto: %d", config.puerto_escucha); // Esto nomas para probar, pero esta de mas y habria que sacarlo
	// TODO
}

void* consola() {
	char *linea;
	char *resultado;
	while (1) {
		linea = readline(">");

		if (!strcmp(linea, "exit")) {
			free(linea);
			break;
		}

		resultado = apiMemoria(linea);
		free(linea);
		puts(resultado);
		free(resultado);
	}
}

void* servidor() {
	log_trace(logger, "Iniciando servidor");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(8082); // Puerto

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	// Estas 2 lineas son para poder reusar un socket en un puerto que ya uso un proceso recientemente terminado
	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, &direccionServidor, sizeof(direccionServidor))) {
		log_error(logger, "Fallo el servidor");
		exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
	}

	listen(servidor, SOMAXCONN);
	log_trace(logger, "Escuchando");

	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion = sizeof(direccionCliente);
	int cliente = accept(servidor, &direccionCliente, &tamanoDireccion);
	log_trace(logger, "Recibi una conexion en %d", cliente);

	send(cliente, "Hola soy Memoria\n", sizeof("Hola soy Memoria\n"), 0); // Mando saludo
	while (1) {
		char* buffer = malloc(17);

		int bytesRecibidos = recv(cliente, buffer, 16, 0);
		if (bytesRecibidos <= 0) {
			log_error(logger, "El cliente se desconecto");
			exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
		}
		buffer[bytesRecibidos] = '\0';
		string_trim(&buffer);
		log_trace(logger, "Me llego el mensaje: %s", buffer);
		if (!strcmp(buffer, "Hola soy Kernel")) {
			send(cliente, "Hola Kernel!\n", sizeof("Hola Kernel!\n"), 0);
			free(buffer);
			break;
		}
		free(buffer);
	}

	while (1) {
		char* buffer = malloc(100);

		int bytesRecibidos = recv(cliente, buffer, 99, 0);
		if (bytesRecibidos <= 0) {
			log_error(logger, "El cliente se desconecto");
			exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
		}
		buffer[bytesRecibidos - 1] = '\0';
		string_trim(&buffer);
		log_trace(logger, "Me llego el mensaje: %s", buffer);
		char* resultado = apiMemoria(buffer);
		send(cliente, resultado, string_length(resultado), 0);
		free(buffer);
		free(resultado);
	}
}

void* cliente(){
	log_trace(logger, "Iniciando cliente");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr("127.0.0.1"); // Direccion IP
	direccionServidor.sin_port = htons(5003); // PUERTO

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	log_trace(logger, "Conectando con servidor (FS)");
	if (connect(cliente, &direccionServidor, sizeof(direccionServidor))) {
		log_error(logger, "No se pudo conectar con el servidor (FS)");
		exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
	}

	send(cliente, "Hola soy Memoria", sizeof("Hola soy Memoria"), 0);
	while (1) {
		char* buffer = malloc(sizeof("Hola soy FS"));

		int bytesRecibidos = recv(cliente, buffer, sizeof("Hola soy FS"), 0);
		if (bytesRecibidos <= 0) {
			log_error(logger, "El cliente se desconecto");
			exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
		}
		log_trace(logger, "Me llegaron %d bytes con el mensaje: %s", bytesRecibidos, buffer);
		if (!strcmp(buffer, "Hola soy FS")) {
			free(buffer);
			buffer = NULL;
			char*  buffer = malloc(sizeof("Hola Memoria!"));
			int bytesRecibidos = recv(cliente, buffer, sizeof("Hola Memoria!"), 0);
			if (bytesRecibidos <= 0) {
				log_error(logger, "El cliente se desconecto");
				exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
			}
			log_trace(logger, "Me llegaron %d bytes con el mensaje: %s", bytesRecibidos, buffer);
			if (!strcmp(buffer, "Hola Memoria!")) {
				// En este momento terminamos todo el "saludo" y salio bien. Seguimos ejecutando fuera del while
				log_trace(logger, "Saludo terminado");
				break;
			}
		}
		free(buffer);
	}
}
