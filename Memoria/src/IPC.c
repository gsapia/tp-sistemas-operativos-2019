#include "IPC.h"
#include "MemoriaPrincipal.h"

int socket_cliente;

struct_select_respuesta selectAFS(struct_select paquete){
	// ----- Provisoriamente uso una respuesta por defecto: -----
	struct_select_respuesta respuesta;
	respuesta.estado = ESTADO_SELECT_OK;
	respuesta.valor = strdup("VALOR");
	respuesta.timestamp = 123456;
	return respuesta;
	// ----- Fin parte provisoria -----
	enviar_select(socket_cliente, paquete);
	return recibir_registro(socket_cliente);
}
enum estados_create createAFS(struct_create paquete){
	// ----- Provisoriamente uso una respuesta por defecto: -----
	return ESTADO_CREATE_OK;
	// ----- Fin parte provisoria -----
	enviar_create(socket_cliente, paquete);
	return recibir_respuesta_create(socket_cliente);
}

void initCliente(){
	log_trace(logger, "Iniciando cliente");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(config.ip_fs); // Direccion IP
	direccionServidor.sin_port = htons(config.puerto_fs); // PUERTO

	log_trace(logger, "Conectando con FS en %s:%d",config.ip_fs,config.puerto_fs);

	socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
	while(connect(socket_cliente, (void*) &direccionServidor, sizeof(direccionServidor))){
		log_trace(logger, "No se pudo conectar con el servidor (FS). Reintentando en 5 segundos.");
		sleep(95); // TODO CAMBIAR POR 5
	}

	//----------------COMIENZO HANDSHAKE----------------

	// Envio primer mensaje diciendo que soy Memoria
	const uint8_t soy = ID_MEMORIA;
	send(socket_cliente, &soy, sizeof(soy), 0);

	// Recibo confirmacion de que el otro extremo es FS
	uint8_t *otro = malloc(sizeof(uint8_t));
	if(!(recv(socket_cliente, otro, sizeof(uint8_t), 0) && *otro == ID_FILESYSTEM)){ // Confirmo que el otro extremo es FS
		// El otro extremo no es FS, asi que cierro la conexion / termino el programa
		log_error(logger, "Error, no pudimos conectar con FileSystem");
		exit(EXIT_FAILURE);
	}
	// El otro extremo es FS realmente asi que ahora enviamos/recibimos los datos necesarios

	// En este caso recibimos el tamaño del value
	recv(socket_cliente, &tamanio_value, sizeof(tamanio_value), 0);

	//-------------------FIN HANDSHAKE------------------

	log_trace(logger, "Me conecte con FS!");

	/*
	 * TODO:
	 * Enviar datos
	 */
}

void closeCliente(){
	close(socket_cliente); // No me olvido de cerrar el socket que ya no voy a usar
}

void servidor() {
	log_trace(logger, "Iniciando servidor");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(config.puerto_escucha); // Puerto

	int socket_servidor = socket(AF_INET, SOCK_STREAM, 0);

	if (bind(socket_servidor, (void*) &direccionServidor, sizeof(direccionServidor))) {
		log_trace(logger, "Fallo el servidor");
		exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
	}

	listen(socket_servidor, SOMAXCONN);
	log_trace(logger, "Escuchando en el puerto %d...",config.puerto_escucha);

	bool conectado = false;

	int socket_cliente;

	while(!conectado){
		struct sockaddr_in direccionCliente;
		unsigned int tamanoDireccion = sizeof(direccionCliente);
		socket_cliente = accept(socket_servidor, (void*) &direccionCliente, &tamanoDireccion);
		printf("Recibi una conexion en %d\n", socket_cliente);


		//----------------COMIENZO HANDSHAKE----------------

		// Recibo quien es el otro extremo
		uint8_t *otro = malloc(sizeof(uint8_t));

		if(!(recv(socket_cliente, otro, sizeof(uint8_t), 0) && *otro == ID_KERNEL)){ // Confirmo que el otro extremo es Kernel
			// El otro extremo no es Kernel, cierro la conexion / termino el programa
			log_error(logger, "Recibi una conexion de alguien que no es Kernel.");
			close(socket_cliente);
		}
		else{
			// El otro extremo es Kernel realmente
			// Envio confirmacion de que soy Memoria
			const uint8_t soy = ID_MEMORIA;
			send(socket_cliente, &soy, sizeof(soy), 0);

			// Y ahora entonces le enviamos/recibimos los datos necesarios


			//Y por ahora no necesito enviar/recibir mas nada
			// Asi que el handshake termino y me quedo a la espera de solicitudes de Kernel

			//-------------------FIN HANDSHAKE------------------

			conectado = true;
			log_trace(logger, "Me conecte con Kernel!");
		}
	}


	/*
	 * TODO:
	 * Esperar solicitudes
	 */

	while(1){
		// Recibo el codigo de op
		uint8_t cod_op;
		if(!recv(socket_cliente, &cod_op, sizeof(uint8_t), 0)){ // Problema, recv es no bloqueante, asi que estoy en espera activa hasta
													  	 // que se desconecte el cliente o reciba algo. Deberiamos usar select()?
			log_trace(logger, "El cliente se desconecto");
			break;
		}

		switch (cod_op) {
			case SELECT:
			{
				log_trace(logger, "Recibi un SELECT");
				struct_select paquete = recibir_select(socket_cliente);

				/*
				 * Depues haria lo que tenga que hacer con esta struct ya cargada
				 */
				printf("Comando recibido: SELECT %s %d\n\n", paquete.nombreTabla, paquete.key);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
			break;
			case INSERT:
			{
				log_trace(logger, "Recibi un INSERT");
				struct_insert paquete = recibir_insert(socket_cliente);

				/*
				 * Depues haria lo que tenga que hacer con esta struct ya cargada
				 */
				printf("Comando recibido: INSERT %s %d \"%s\"\n\n", paquete.nombreTabla, paquete.key, paquete.valor);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
				free(paquete.valor);
			}
			break;
			case CREATE:
				break;
			case DESCRIBE:
			{
				log_trace(logger, "Recibi un DESCRIBE");
				struct_describe paquete = recibir_describe(socket_cliente);

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
			case JOURNAL:
				break;
			default:
				log_trace(logger, "Recibi una operacion invalida...");
		}
	}
	close(socket_cliente); // No me olvido de cerrar el socket que ya no voy a usar mas
	close(socket_servidor); // No me olvido de cerrar el socket que ya no voy a usar mas
}
