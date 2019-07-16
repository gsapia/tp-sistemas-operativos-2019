#include "IPC.h"
#include "Kernel.h"
#include "serializacion.h"
#include "Memorias.h"

int conectar(char* ip, uint16_t puerto){
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(ip); //Direccion IP
	direccionServidor.sin_port = htons(puerto); // Puerto al que me conecto (Memoria)

	//log_trace(logger, "Conectando con Memoria en %s:%d",config.ip_memoria,config.puerto_memoria);

	int socket_cliente = socket(AF_INET, SOCK_STREAM, 0);//Pedimos un socket enviandole parametros que especifica que utilizamos protocolo TCP/ IP
	while(connect(socket_cliente, (void*) &direccionServidor, sizeof(direccionServidor))){
		log_trace(logger, "No se pudo conectar con el servidor (Memoria). Reintentando en 5 segundos."); // TODO: Mejorar esto, limitar los reintentos
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

void addMetadata(char* nombre_tabla, struct_describe_respuesta* describe){
	t_metadata *metadata_tabla;
	if(!dictionary_has_key(metadata, nombre_tabla)){
		metadata_tabla = malloc(sizeof(t_metadata));
		dictionary_put(metadata, nombre_tabla, metadata_tabla);
	}
	else{
		metadata_tabla = dictionary_get(metadata, nombre_tabla);
	}
	metadata_tabla->consistencia = describe->consistencia;
	metadata_tabla->particiones = describe->particiones;
	metadata_tabla->tiempo_compactacion = describe->tiempo_compactacion;
}

void refreshMetadata(){
	t_memoria* memoria = obtener_memoria_random_del_pool();
	log_trace(logger, "Actualizando metadata de tablas a traves de la memoria %d", memoria->numero);

	struct_describe_global_respuesta respuesta = describeGlobalAMemoria(memoria);

	if(respuesta.estado == ESTADO_DESCRIBE_OK){
		if(!metadata)
			metadata = dictionary_create();
		dictionary_clean_and_destroy_elements(metadata, free);

		dictionary_iterator(respuesta.describes, (void(*)(char*,void*)) addMetadata);

		log_info(logger, "Metadata de tablas actualizada. Hay %d tablas conocidas.", dictionary_size(metadata));
	}
	else{
		log_warning(logger, "Hubo un error al pedir la metadata de las tablas.");
	}
	dictionary_destroy_and_destroy_elements(respuesta.describes, free);
	free(memoria);
}

void updateMetadata(){
	while(1){
		pthread_mutex_lock(&mutex_metadata);
		refreshMetadata();
		pthread_mutex_unlock(&mutex_metadata);
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


// TODO: En las siguientes funciones evaluar que pasa si no esta online esa memoria:
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
enum estados_create createAMemoria(struct_create paquete, t_memoria* memoria){
	int socket_cliente = conectar(memoria->IP, memoria->puerto);
	enviar_create(socket_cliente, paquete);
	enum estados_create respuesta = recibir_respuesta_create(socket_cliente);
	close(socket_cliente);
	return respuesta;
}
struct_describe_respuesta describeAMemoria(struct_describe paquete, t_memoria * memoria){
	int socket_cliente = conectar(memoria->IP, memoria->puerto);

	enviar_describe(socket_cliente, paquete);
	struct_describe_respuesta respuesta = recibir_respuesta_describe(socket_cliente);

	close(socket_cliente);
	return respuesta;
}
struct_describe_global_respuesta describeGlobalAMemoria(t_memoria* memoria){
	int socket_cliente = conectar(memoria->IP, memoria->puerto);

	const uint8_t cod_op = DESCRIBE_GLOBAL;
	send(socket_cliente, &cod_op, sizeof(cod_op), 0);

	struct_describe_global_respuesta respuesta = recibir_respuesta_describe_global(socket_cliente);
	close(socket_cliente);
	return respuesta;
}
enum estados_journal journalMemoria(t_memoria* memoria){
	int socket_cliente = conectar(memoria->IP, memoria->puerto);
	enviar_journal(socket_cliente);
	enum estados_journal respuesta = recibir_respuesta_journal(socket_cliente);
	close(socket_cliente);
	return respuesta;
}

enum estados_drop dropTabla (struct_drop paquete, t_memoria* memoria){
	int socket_cliente = conectar(memoria->IP, memoria->puerto);
	enviar_drop(socket_cliente, paquete);
	enum estados_drop respuesta = recibir_respuesta_drop(socket_cliente);
	close(socket_cliente);
	return respuesta;
}
