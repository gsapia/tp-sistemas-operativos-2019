#include "IPC.h"
#include "MemoriaPrincipal.h"
#include "API.h"
#include "Gossiping.h"
#include "Config.h"
#include "Misc.h"

//int socket_cliente;
pthread_mutex_t mutex_fs; // Debemos evitar condiciones de carrera en requests a FS

enum id_proceso handshake(int socket_conexion){
	// Envio primer mensaje diciendo que soy Memoria
	const uint8_t soy = ID_MEMORIA;
	send(socket_conexion, &soy, sizeof(soy), 0);

	// Recibo quien es el otro extremo
	uint8_t otro;
	if(!recv(socket_conexion, &otro, sizeof(otro), 0)){
		// No recibimos nada, algo malo paso
		log_error(logger, "ERROR en servidor");
		close(socket_conexion);
		return 0;
	}

	return otro;
}

int connectFS(){
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(config.ip_fs); // Direccion IP
	direccionServidor.sin_port = htons(config.puerto_fs); // PUERTO

	int socket_fs = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(socket_fs, (void*) &direccionServidor, sizeof(direccionServidor)) || handshake(socket_fs) != ID_FILESYSTEM){
		return 0;
	}

	// Una vez conectados, recibimos el tamanio del value
	recv(socket_fs, &tamanio_value, sizeof(tamanio_value), 0);

	return socket_fs;
}

void operacionAFS(void (*operacion)(int socket)){
	pthread_mutex_lock(&mutex_fs);
	int socket_cliente = connectFS();
	if(socket_cliente){
		operacion(socket_cliente);
		close(socket_cliente);
	}
	else{
		log_warning(logger, "No nos podemos conectar con FS.");
	}
	msleep(config.retardo_acc_fs);
	pthread_mutex_unlock(&mutex_fs);
}

struct_select_respuesta selectAFS(struct_select paquete){
	// ----- Provisoriamente uso una respuesta por defecto: -----
	//struct_select_respuesta respuesta;
	//respuesta.estado = ESTADO_SELECT_OK;
	//respuesta.valor = strdup("VALOR");
	//respuesta.timestamp = 123456;
	//return respuesta;
	// ----- Fin parte provisoria -----
	struct_select_respuesta respuesta = { .estado = ESTADO_SELECT_ERROR_OTRO };
	void operacion(int socket){
		enviar_select(socket, paquete);
		respuesta = recibir_registro(socket);
	}

	operacionAFS(operacion);
	return respuesta;
}

enum estados_insert insertAFS(struct_insert paquete){
	// ----- Provisoriamente uso una respuesta por defecto: -----
	// return ESTADO_INSERT_OK;
	// ----- Fin parte provisoria -----
	enum estados_insert respuesta = respuesta = ESTADO_INSERT_ERROR_OTRO;
	void operacion(int socket){
		enviar_insert_ts(socket, paquete);
		respuesta = recibir_respuesta_insert(socket);
	}

	operacionAFS(operacion);
	return respuesta;
}

enum estados_create createAFS(struct_create paquete){
	// ----- Provisoriamente uso una respuesta por defecto: -----
	//return ESTADO_CREATE_OK;
	// ----- Fin parte provisoria -----
	enum estados_create respuesta;
	void operacion(int socket){
		enviar_create(socket, paquete);
		respuesta = recibir_respuesta_create(socket);
	}

	operacionAFS(operacion);
	return respuesta;
}

struct_describe_respuesta describeAFS(struct_describe paquete){
	// ----- Provisoriamente uso una respuesta por defecto: -----
//	struct_describe_respuesta respuesta;
//	respuesta.estado = ESTADO_DESCRIBE_OK;
//	respuesta.consistencia = SC;
//	respuesta.particiones = 5;
//	respuesta.tiempo_compactacion = 60000;
	// return respuesta;
	// ----- Fin parte provisoria -----

	struct_describe_respuesta respuesta = { .estado = ESTADO_DESCRIBE_ERROR_OTRO };
	void operacion(int socket){
		enviar_describe(socket, paquete);
		respuesta = recibir_respuesta_describe(socket);
	}

	operacionAFS(operacion);
	return respuesta;
}
struct_describe_global_respuesta describeGlobalAFS(){
	// ----- Provisoriamente uso una respuesta por defecto: -----
//	struct_describe_global_respuesta respuesta;
//	respuesta.estado = ESTADO_DESCRIBE_OK;
//
//	struct_describe_respuesta* describe1 = malloc(sizeof(struct_describe_respuesta));
//	describe1->consistencia = SC;
//	describe1->particiones = 5;
//	describe1->tiempo_compactacion = 60000;
//
//	struct_describe_respuesta* describe2 = malloc(sizeof(struct_describe_respuesta));
//	describe2->consistencia = EC;
//	describe2->particiones = 7;
//	describe2->tiempo_compactacion = 60000;
//
//	respuesta.describes = dictionary_create();
//	dictionary_put(respuesta.describes, strdup("TABLA1"), describe1);
//	dictionary_put(respuesta.describes, strdup("TABLA2"), describe2);
//	return respuesta;
	// ----- Fin parte provisoria -----

	struct_describe_global_respuesta respuesta;
	respuesta.estado = ESTADO_DESCRIBE_ERROR_OTRO;
	respuesta.describes = dictionary_create();
	void operacion(int socket){
		enviar_describe_global(socket);
		respuesta = recibir_respuesta_describe_global(socket);
	}

	operacionAFS(operacion);
	return respuesta;
}

enum estados_drop dropAFS(struct_drop paquete){
	// ----- Provisoriamente uso una respuesta por defecto: -----
	// return ESTADO_DROP_OK;
	// ----- Fin parte provisoria -----
	enum estados_drop respuesta = ESTADO_DESCRIBE_ERROR_OTRO;
	void operacion(int socket){
		enviar_drop(socket, paquete);
		respuesta = recibir_respuesta_drop(socket);
	}

	operacionAFS(operacion);
	return respuesta;
}



void initCliente(){
	log_info(logger, "Conectando con FS en %s:%d",config.ip_fs,config.puerto_fs);
	int socket_cliente = connectFS();
	while(!socket_cliente){
		socket_cliente = connectFS();
		log_warning(logger, "No se pudo conectar con el servidor (FS). Reintentando en 5 segundos.");
		sleep(5);
	}
	close(socket_cliente);
	log_info(logger, "Me conecte con FS y obutve el tamanio maximo de value.");
}

void closeCliente(){
	//close(socket_cliente); // No me olvido de cerrar el socket que ya no voy a usar
}

bool kernel_conectado = false;

t_list* intercambiar_tabla_gossiping(t_memoria memoria){
	// Intentamos conectarnos con la memoria
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(memoria.IP); // Direccion IP
	direccionServidor.sin_port = htons(memoria.puerto); // PUERTO

	int socket_memoria = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(socket_memoria, (void*) &direccionServidor, sizeof(direccionServidor)) || handshake(socket_memoria) != ID_MEMORIA){
		return NULL;
	}

	// Primero enviamos nuestra tabla
	enviar_tabla_gossiping(socket_memoria, tabla_gossiping);

	// Si el otro extremo es una memoria, entonces tambien recibimos
	t_list* tabla = recibir_tabla_gossiping(socket_memoria);

	close(socket_memoria);

	return tabla;
}

void memoria_handler(int *socket_memoria){
	// Enviamos y recibimos las tablas
	enviar_tabla_gossiping(*socket_memoria, tabla_gossiping);
	t_list* tabla = recibir_tabla_gossiping(*socket_memoria);
	agregar_memorias(tabla);

	list_destroy_and_destroy_elements(tabla, free);
	free(socket_memoria);
}

void kernel_handler(int *socket_cliente){
	int socket_kernel = *socket_cliente;
	free(socket_cliente);
	while(1){
		// Recibo el codigo de operacion
		uint8_t cod_op;
		if(!recv(socket_kernel, &cod_op, sizeof(uint8_t), 0)){
			break;
		}

		switch (cod_op) {
			case SELECT:
			{
				struct_select paquete = recibir_select(socket_kernel);
				log_info(logger, "Recibi un SELECT %s %d", paquete.nombreTabla, paquete.key);

				struct_select_respuesta registro = selects(paquete.nombreTabla, paquete.key);
				enviar_registro(socket_kernel, registro);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
			break;
			case INSERT:
			{
				struct_insert paquete = recibir_insert(socket_kernel);
				log_info(logger, "Recibi un INSERT %s %d \"%s\"", paquete.nombreTabla, paquete.key, paquete.valor);

				enum estados_insert estado = insert(paquete.nombreTabla, paquete.key, paquete.valor);
				responder_insert(socket_kernel, estado);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
				free(paquete.valor);
			}
			break;
			case CREATE:
			{
				struct_create paquete = recibir_create(socket_kernel);
				log_info(logger, "Recibi un CREATE %s %s %d %lld", paquete.nombreTabla, consistenciaAString(paquete.consistencia), paquete.particiones, paquete.tiempoCompactacion);

				enum estados_create estado = create(paquete.nombreTabla, paquete.consistencia, paquete.particiones, paquete.tiempoCompactacion);
				responder_create(socket_kernel, estado);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
			break;
			case DESCRIBE:
			{
				struct_describe paquete = recibir_describe(socket_kernel);
				log_info(logger, "Recibi un DESCRIBE %s", paquete.nombreTabla);

				struct_describe_respuesta registro;
				registro = describe(paquete.nombreTabla);

				enviar_respuesta_describe(socket_kernel, registro);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
			break;
			case DESCRIBE_GLOBAL:
			{
				log_info(logger, "Recibi un DESCRIBE");

				struct_describe_global_respuesta respuesta;

				respuesta = describe_global();

				enviar_respuesta_describe_global(socket_kernel, respuesta);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				dictionary_destroy_and_destroy_elements(respuesta.describes, free);
			}
			break;
			case DROP:
			{
				struct_drop paquete = recibir_drop(socket_kernel);
				log_info(logger, "Recibi un DROP %s", paquete.nombreTabla);

				enum estados_drop respuesta = drop(paquete.nombreTabla);

				responder_drop(socket_kernel, respuesta);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
			break;
			case JOURNAL:
			{
				log_info(logger, "Recibi un JOURNAL");

				enum estados_journal respuesta = journal();

				responder_journal(socket_kernel, respuesta);
			}
			break;
			case GOSSIP:
			{
				log_info(logger, "GOSSIPING: Intercambiando tabla con Kernel");
				enviar_tabla_gossiping(socket_kernel, tabla_gossiping);
			}
			break;
			default:
				log_warning(logger, "Recibi una operacion invalida...");
		}
	}

	close(socket_kernel); // No me olvido de cerrar el socket que ya no voy a usar mas
}

void servidor() {
	log_trace(logger, "Iniciando servidor");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(config.puerto_escucha); // Puerto

	int socket_servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(socket_servidor, (void*) &direccionServidor, sizeof(direccionServidor))) {
		log_error(logger, "Fallo el servidor");
		exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
	}

	listen(socket_servidor, SOMAXCONN);
	log_info(logger, "Escuchando en el puerto %d...",config.puerto_escucha);

	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion = sizeof(direccionCliente);
	while(1){
		int socket_conexion = accept(socket_servidor, (void*) &direccionCliente, &tamanoDireccion);
		enum id_proceso otro = handshake(socket_conexion);
		switch (otro) {
		case ID_KERNEL:
		{
			int *socket_cliente = malloc(sizeof(int));
			*socket_cliente = socket_conexion;

			// Mando la ejecucion a un hilo deatacheable
			pthread_t hilo_kernel;
			while(pthread_create(&hilo_kernel, NULL, (void*)kernel_handler, socket_cliente)){
				log_error(logger, "Error creando hilo kernel_handler");
				sleep(5);
			}
			pthread_detach(hilo_kernel);
		}
		break;
		case ID_MEMORIA:
		{
			int *socket_cliente = malloc(sizeof(int));
			*socket_cliente = socket_conexion;

			log_trace(logger, "GOSSIPING: Intercambiando tablas por peticion de una memoria en %s ...", inet_ntoa(direccionCliente.sin_addr));

			// Mando la ejecucion a un hilo deatacheable
			pthread_t hilo_memoria;
			while(pthread_create(&hilo_memoria, NULL, (void*)memoria_handler, socket_cliente)){
				log_error(logger, "Error creando hilo memoria_handler");
				sleep(5);
			}
			pthread_detach(hilo_memoria);
		}
		break;
		default:
			log_warning(logger, "Recibi una conexion de alguien que no es ni Kernel ni Memoria.");
			close(socket_conexion);
			break;
		}
	}
}
