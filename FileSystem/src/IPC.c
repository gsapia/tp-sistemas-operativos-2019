#include "LFS.h"
#include "serializacion.c"
void* servidor(uint16_t puerto_escucha, int tamValue){
	log_trace(logger, "Iniciando servidor");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(puerto_escucha);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor,(void*) &direccionServidor,sizeof(direccionServidor))){
		log_error(logger, "Fallo el servidor");
		exit(EXIT_FAILURE); ///
	}

	listen(servidor,SOMAXCONN);
	log_trace(logger, "Escuchandoen el puerto &d...", puerto_escucha);

	bool conectado = false;

	while(!conectado){
		struct sockaddr_in direccionCliente;
		unsigned int tamanoDireccion = sizeof(direccionCliente);
		int cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
		log_trace(logger, "Recibi una conexion en %d", cliente);

		//COMIENZO HANDSHAKE

		uint8_t *otro = malloc(sizeof(uint8_t));
		if(!(recv(cliente, otro, sizeof(uint8_t), 0) && otro == 2)){ // Confirmo si es Memoria
			log_error(logger, "Recibi una conexion de alguien que no es Memoria.");
			close(cliente);
		}else{
			const uint8_t soy = 3;
			send(cliente, &soy, sizeof(soy), 0); // Le hacemos saber que somos FS
			send(cliente, &tamValue, sizeof(tamValue), 0); // Le envio a memoria el valor que necesita (Tamaño del Value)
			//TERMINO HANSHAKE
			conectado = true;
			log_trace(logger, "¡Me conecte a Memoria!");
		}

	}

	//Espero Solicitudes
	while(1){
		struct sockaddr_in direccionCliente;
		unsigned int tamanoDireccion = sizeof(direccionCliente);
		int cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
		log_trace(logger, "Recibi una conexion en %d", cliente);
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



				printf("Comando recibido: SELECT %s %d\n\n", paquete.nombreTabla, paquete.key);

				free(paquete.nombreTabla);
				}
				break;
			case INSERT:
			{
				log_trace(logger, "Recibi un INSERT");
				struct_insert paquete = recibir_insert(cliente);


				//Depues haria lo que tenga que hacer con esta struct ya cargada

				printf("Comando recibido: INSERT %s %d \"%s\"\n\n", paquete.nombreTabla, paquete.key, paquete.valor);

				free(paquete.nombreTabla);
				free(paquete.valor);
			}
				break;
			case CREATE:
				break;
			case DESCRIBE:
				{
				log_trace(logger, "Recibi un DESCRIBE");
				struct_describe paquete = recibir_describe(cliente);

				/*
				 * Depues haria lo que tenga que hacer con esta struct ya cargada
				 */
				printf("Comando recibido: DESCRIBE %s\n\n", paquete.nombreTabla);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
				}
				break;
				case DROP:
					break;
		}
	}
	close(servidor);
}


