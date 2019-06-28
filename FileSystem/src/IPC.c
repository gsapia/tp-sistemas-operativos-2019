#include "LFS.h"

struct_select_respuesta selects(char* nombreTabla, u_int16_t key);

void* servidor(argumentos_servidor* args){
	uint16_t puerto_escucha = args->puerto_escucha;
	uint16_t tamValue = args->tamValue;
	log_trace(logger, "Iniciando servidor...");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(puerto_escucha);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor,(void*) &direccionServidor,sizeof(direccionServidor))){
		log_error(logger, "Fallo el servidor");
		exit(EXIT_FAILURE);
	}

	listen(servidor,SOMAXCONN);
	log_trace(logger, "Escuchando en el puerto %d...", puerto_escucha);

	while(1){
		bool conectado = false;
		int cliente;
		while(!conectado){
			struct sockaddr_in direccionCliente;
			unsigned int tamanoDireccion = sizeof(direccionCliente);
			cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
			log_trace(logger, "Recibi una conexion en %d", cliente);

			//COMIENZO HANDSHAKE

			uint8_t *otro = malloc(sizeof(uint8_t));
			if(!(recv(cliente, otro, sizeof(uint8_t), 0) && *otro == ID_MEMORIA)){ // Confirmo si es Memoria
				log_error(logger, "Recibi una conexion de alguien que no es Memoria.");
				close(cliente);
			}else{
				const uint8_t soy = ID_FILESYSTEM;
				send(cliente, &soy, sizeof(soy), 0); // Le hacemos saber que somos FS
				send(cliente, &tamValue, sizeof(tamValue), 0); // Le envio a memoria el valor que necesita (Tamaño del Value)
				//TERMINO HANSHAKE
				conectado = true;
				log_trace(logger, "¡Me conecte a Memoria!");
			}

		}

		// Conexion con una memoria establecida, asi que ahora creamos un hilo para atender sus peticiones
		int *socket_cliente = malloc(sizeof(int));
		*socket_cliente = cliente;

		// Mando la ejecucion a un hilo deatacheable
		pthread_t hilo;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		while(pthread_create(&hilo, &attr, (void*)memoria_handler, socket_cliente)){
			sleep(5);
			log_info(logger, "Error creando hilo cliente");
		}
		pthread_attr_destroy(&attr);
		}

	close(servidor);
}

void memoria_handler(int *socket_cliente){
	int cliente = *socket_cliente;
	free(socket_cliente);

	while(1){
		uint8_t cod_op;

		if(!recv(cliente, &cod_op, sizeof(uint8_t), 0)){
			log_trace(logger, "El cliente se desconecto");
			break;
		}

		switch(cod_op){
			case SELECT:
			{
				log_trace(logger, "Recibi un SELECT");
				struct_select paquete = recibir_select(cliente);

				struct_select_respuesta registro;
				registro = selects(paquete.nombreTabla, paquete.key);

				printf("Comando recibido: SELECT %s %d\n\n", paquete.nombreTabla, paquete.key);

				enviar_registro(cliente, registro);

				free(paquete.nombreTabla);
			}
			break;
			case INSERT:
			{
				log_trace(logger, "Recibi un INSERT");
				struct_insert paquete = recibir_insert(cliente);

				enum estados_insert estado= insert(paquete.nombreTabla, paquete.key, paquete.valor, paquete.timestamp);

				responder_insert(cliente, estado);

				printf("Comando recibido: INSERT %s %d \"%s\"\n\n", paquete.nombreTabla, paquete.key, paquete.valor);

				free(paquete.nombreTabla);
				free(paquete.valor);
			}
			break;
			case CREATE:
			{
				puts("Recibi un CREATE");
				struct_create paquete = recibir_create(cliente);
				char* consistencia;
				switch (paquete.consistencia) {
					case SC:
						consistencia = "SC";
						break;
					case SHC:
						consistencia = "SHC";
						break;
					case EC:
						consistencia = "EC";
						break;
					default:
						consistencia = ""; // TODO esto esta como el orto. Lo ideal es redefinir create para levantar un numero y no un char*
						break;
				}

				int estado = create(paquete.nombreTabla, consistencia, paquete.particiones, paquete.tiempoCompactacion);
				responder_create(cliente, estado);

				printf("Comando recibido: CREATE %s %d %d %d\n\n", paquete.nombreTabla, paquete.consistencia, paquete.particiones, paquete.tiempoCompactacion);
				free(paquete.nombreTabla);
			}
			break;
			case DESCRIBE:
			{
				log_trace(logger, "Recibi un DESCRIBE");
				struct_describe paquete = recibir_describe(cliente);

				struct_describe_respuesta registro;
				registro = describe(paquete.nombreTabla);

				enviar_respuesta_describe(cliente, registro);

				printf("Comando recibido: DESCRIBE %s\n\n", paquete.nombreTabla);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
				break;
			case DESCRIBE_GLOBAL:
			{
				log_trace(logger, "Recibi un DESCRIBE GLOBAL");

				struct_describe_global_respuesta respuesta;

				respuesta = describe_global();

				enviar_respuesta_describe_global(cliente, respuesta);
				dictionary_destroy_and_destroy_elements(respuesta.describes, free);

			}
			break;
			case DROP:
			{
				puts("Recibi un DROP");
				struct_drop paquete = recibir_drop(cliente);

				//Hacer algo con el paquete

				printf("Comando recibido: DROP %s\n\n", paquete.nombreTabla);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
			break;
		}
	}
	close(cliente);
}

