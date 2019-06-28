#include "IPC.h"
#include "Kernel.h"
#include "serializacion.h"
#include "Memorias.h"

int conectar(char* ip, uint16_t puerto){
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(ip); //Direccion IP
	direccionServidor.sin_port = htons(puerto); // Puerto al que me conecto (Memoria)
	// TODO: Puerto e IP deberian salir del archivo de configuracion

	//log_trace(logger, "Conectando con Memoria en %s:%d",config.ip_memoria,config.puerto_memoria);

	int socket_cliente = socket(AF_INET, SOCK_STREAM, 0);//Pedimos un socket enviandole parametros que especifica que utilizamos protocolo TCP/ IP
	while(connect(socket_cliente, (void*) &direccionServidor, sizeof(direccionServidor))){
		log_trace(logger, "No se pudo conectar con el servidor (Memoria). Reintentando en 5 segundos.");
		sleep(5);
	}


	//************ Conexion Kernel - Memoria (HandShake)***************


	//Envio un primer mensaje

	const uint8_t k = ID_KERNEL;
	send(socket_cliente,&k, sizeof(k),0 );

	//Recibo confirmacion de que el otro extremo es Memoria

	uint8_t *otro= malloc (sizeof(uint8_t));

	if(!(recv(socket_cliente, otro, sizeof(uint8_t ),0) && *otro == ID_MEMORIA)) // Confirmo que el otro extremo es Memoria
	{
		//Si el otro extremo no es Memoria, cierro la conexion tirando un log y termino el programa.
		log_error(logger,"Error, me conecte con alguien que no es memoria.");
		close(socket_cliente);
		return 0;

	}
	free(otro);

	//Fin de HandShake - Ahora podemos realizar solicitudes a Memoria.
	return socket_cliente;
}

void updateMetadata(){
	while(1){
		t_memoria* memoria = obtener_memoria_random_del_pool();
		log_trace(logger, "Actualizando metadata de tablas a travez de la memoria %d", memoria->numero);
		int socket = conectar(memoria->IP, memoria->puerto);
		if(!socket){
			log_trace(logger, "No nos pudimos conectar con la memoria para pedir su metadata.");
		}
		else{
			// Mandamos un DESCRIBE global
			const uint8_t cod_op = DESCRIBE_GLOBAL;
			send(socket, &cod_op, sizeof(cod_op), 0);

			struct_describe_global_respuesta respuesta = recibir_respuesta_describe_global(socket);

			if(respuesta.estado == ESTADO_DESCRIBE_OK){
				// TODO: Mutex para la metadata.
				metadata = dictionary_create();

				void iterador(char* nombre_tabla, struct_describe_respuesta* describe){
					t_metadata *metadata_tabla = malloc(sizeof(t_metadata));
					metadata_tabla->consistencia = describe->consistencia;
					metadata_tabla->particiones = describe->particiones;
					metadata_tabla->tiempo_compactacion = describe->tiempo_compactacion;

					dictionary_put(metadata, nombre_tabla, metadata_tabla);
				}

				dictionary_iterator(respuesta.describes, (void(*)(char*,void*)) iterador);

				log_info(logger, "Metadata de tablas actualizada. Hay %d tablas conocidas.", dictionary_size(metadata));
			}
			else{
				log_warning(logger, "Hubo un error al pedir la metadata de las tablas.");
			}
			dictionary_destroy_and_destroy_elements(respuesta.describes, free);
		}
		usleep(config.refresh_metadata * 1000);
	}

}

void initCliente(){
	log_trace(logger, "Iniciando socket_cliente kernel");


	int socket_cliente = conectar(config.ip_memoria, config.puerto_memoria);
	while(!socket_cliente){
		sleep(5);
		socket_cliente = conectar(config.ip_memoria, config.puerto_memoria);
	}

	log_trace(logger,"Contectado a Memoria !");

	/*
	// Recibimos IPs del resto de memorias
	memorias = list_create();

	// Primero aniadimos la que conocemos
	t_memoria* memoria = malloc(sizeof(t_memoria));
	memoria->IP = config.ip_memoria;
	memoria->puerto = config.puerto_memoria;
	list_add(memorias, memoria);

	// Ahora el resto
	uint16_t cantidad_memorias;
	recv(socket_cliente, &cantidad_memorias, sizeof(cantidad_memorias), 0); // Recibo la cantidad de memorias
	list_add(memorias, memoria);

	for(int i = 0; i < cantidad_memorias; i++){
		uint16_t tamanio_ip;
		recv(socket_cliente, &tamanio_ip, sizeof(tamanio_ip), 0); // Recibo el tamanio de la IP
		char* ip = malloc(tamanio_ip);
		recv(socket_cliente, ip, tamanio_ip, 0); // Recibo la IP
		uint16_t puerto;
		recv(socket_cliente, &puerto, sizeof(puerto), 0); // Recibo el puerto

		memoria = malloc(sizeof(t_memoria));
		memoria->IP = ip;
		memoria->puerto = puerto;

		list_add(memorias, memoria);
		log_trace(logger, "Recibi memoria %s:%d", ip, puerto);
	}

	close(socket_cliente);*/
	//**Fin conexion Kernel-Memoria**//

	close(socket_cliente);

	sem_init(&primer_gossip_hecho,0,0);
	// Inciamos el gossiping
	pthread_t hiloGossip;
	if (pthread_create(&hiloGossip, NULL, (void*)gossip, NULL)) {
		log_error(logger, "Hilo gossip: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}
	pthread_detach(hiloGossip);

	sem_wait(&primer_gossip_hecho);
	// Lo mismo con la metadata
	pthread_t hiloMetadata;
	if (pthread_create(&hiloMetadata, NULL, (void*)updateMetadata, NULL)) {
		log_error(logger, "Hilo metadata: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}
	pthread_detach(hiloMetadata);


} //End Cliente



void closeCliente(){
	//close(socket_cliente); // No me olvido de cerrar el socket que ya no voy a usar
}

struct_select_respuesta selectAMemoria(struct_select paquete, t_memoria* memoria){
	log_debug(logger, "Enviando request a la memoria %d (%s:%d)", memoria->numero, memoria->IP, memoria->puerto);
	int socket_cliente = conectar(memoria->IP, memoria->puerto);
	enviar_select(socket_cliente, paquete);
	struct_select_respuesta respuesta = recibir_registro(socket_cliente);
	close(socket_cliente);
	return respuesta;
}
enum estados_insert insertAMemoria(struct_insert paquete, t_memoria* memoria){
	log_debug(logger, "Enviando request a la memoria %d (%s:%d)", memoria->numero, memoria->IP, memoria->puerto);
	int socket_cliente = conectar(memoria->IP, memoria->puerto);
	enviar_insert(socket_cliente, paquete);
	enum estados_insert respuesta = recibir_respuesta_insert(socket_cliente);
	close(socket_cliente);
	return respuesta;
}
enum estados_create createAMemoria(struct_create paquete){
	int socket_cliente = conectar(config.ip_memoria, config.puerto_memoria); // Teoricamente puede ir a cualquier memoria
	enviar_create(socket_cliente, paquete);
	enum estados_create respuesta = recibir_respuesta_create(socket_cliente);
	close(socket_cliente);
	return respuesta;
}
