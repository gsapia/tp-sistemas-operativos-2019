#include "Memoria.h"
#include "API.h"

void* consola();
void* servidor();

int main(void) {
	logger = log_create("Memoria.log", "Memoria", 1, LOG_LEVEL_TRACE);
	log_info(logger, "Hola Soy Memoria");
	pthread_t hiloConsola;
	if(pthread_create(&hiloConsola, NULL, consola, NULL)){
		log_error(logger, "Hilo consola: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}

	pthread_t hiloServidor;
	if(pthread_create(&hiloServidor, NULL, servidor, NULL)){
		log_error(logger, "Hilo servidor: Error - pthread_create()");
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

void* consola(){
	char *linea;
	char *resultado;
	while(1) {
		linea = readline(">");

		if(!strcmp(linea,"exit")){
			free(linea);
			break;
		}

		resultado = apiMemoria(linea);
		free(linea);
		puts(resultado);
		free(resultado);
	}
}

void* servidor(){
	log_trace(logger, "Iniciando servidor");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(8080);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor,&direccionServidor,sizeof(direccionServidor))){
		log_error(logger, "Fallo el servidor");
		exit(EXIT_FAILURE); ///
	}

	listen(servidor,SOMAXCONN);
	log_trace(logger, "Escuchando");

	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion = sizeof(direccionCliente);
	int cliente = accept(servidor, &direccionCliente, &tamanoDireccion);
	log_trace(logger, "Recibi una conexion en %d", cliente);


	send(cliente, "Hola soy Memoria\n", sizeof("Hola soy Memoria\n"), 0);
	while(1){
		char* buffer = malloc(17);

		int bytesRecibidos = recv(cliente, buffer, 16, 0);
		if(bytesRecibidos < 0){
			log_error(logger, "El cliente se desconecto");
			exit(EXIT_FAILURE); ///
		}
		buffer[bytesRecibidos] = '\0';
		string_trim(&buffer);
		log_trace(logger, "Me llego el mensaje: %s", buffer);
		if(!strcmp(buffer,"Hola soy Kernel")){
			send(cliente, "Hola Kernel!\n", sizeof("Hola Kernel!\n"), 0);
			free(buffer);
			break;
		}
		free(buffer);
	}

	while(1){
		char* buffer = malloc(100);

		int bytesRecibidos = recv(cliente, buffer, 99, 0);
		if(bytesRecibidos < 0){
			log_error(logger, "El cliente se desconecto");
			exit(EXIT_FAILURE); ///
		}
		buffer[bytesRecibidos-1] = '\0';
		string_trim(&buffer);
		log_trace(logger, "Me llego el mensaje: %s", buffer);
		char* resultado = apiMemoria(buffer);
		send(cliente, resultado, string_length(resultado), 0);
		free(buffer);
		free(resultado);
	}
}

