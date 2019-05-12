#include "IPC.h"

struct_select recibir_select(int cliente){
	struct_select paquete;
	void* buffer;

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(uint16_t));
	recv(cliente, buffer, sizeof(uint16_t), 0);
	paquete.tamanio_nombre = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("El nombre de tabla es de %d bytes\n", paquete.tamanio_nombre);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(paquete.tamanio_nombre);
	recv(cliente, buffer, paquete.tamanio_nombre, 0);
	paquete.nombreTabla = malloc(paquete.tamanio_nombre);
	memcpy(paquete.nombreTabla, buffer, paquete.tamanio_nombre);
	printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la key
	buffer = malloc(sizeof(paquete.key));
	recv(cliente, buffer, sizeof(paquete.key), 0);
	paquete.key = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("La key es %d\n", paquete.key);
	free(buffer);

	puts("Listo, recibi el paquete completo!\n");

	return paquete;

	// Para tener en cuenta: El recv recibe un maximo de bytes como diga el tercer parametro,
	// pero puede recibir menos por alguna circunstancia no esperada (si llegaron menos bytes de los que esperaba).
	// La cantidad de bytes recibidos es el valor que devuelve recv, por lo que podemos almacenar y usar ese valor
	// para controlar posibles errores y abortar la operacion.
}

struct_insert recibir_insert(int cliente){
	struct_insert paquete;
	void* buffer = NULL;

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(paquete.tamanio_nombre));
	recv(cliente, buffer, sizeof(paquete.tamanio_nombre), 0);
	paquete.tamanio_nombre = *((uint16_t*)buffer);
	printf("El nombre de tabla es de %d bytes\n", paquete.tamanio_nombre);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(paquete.tamanio_nombre);
	recv(cliente, buffer, paquete.tamanio_nombre, 0);
	paquete.nombreTabla = malloc(paquete.tamanio_nombre);
	memcpy(paquete.nombreTabla, buffer, paquete.tamanio_nombre);
	printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la key
	buffer = malloc(sizeof(paquete.key));
	recv(cliente, buffer, sizeof(uint16_t), 0);
	paquete.key = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("La key es %d\n", paquete.key);
	free(buffer);

	// Por ultimo el valor
	buffer = malloc(sizeof(uint16_t));
	recv(cliente, buffer, sizeof(uint16_t), 0);
	paquete.tamanio_valor = *((uint16_t*)buffer);
	printf("El valor es de %d bytes\n", paquete.tamanio_valor);
	free(buffer);

	buffer = malloc(paquete.tamanio_valor);
	recv(cliente, buffer, paquete.tamanio_valor, 0);
	paquete.valor = malloc(paquete.tamanio_valor);
	memcpy(paquete.valor, buffer, paquete.tamanio_valor);
	printf("El valor es \"%s\"\n", paquete.valor);
	free(buffer);

	puts("Listo, recibi el paquete completo!\n");

	return paquete;
}

struct_describe recibir_describe(int cliente){
	struct_describe paquete;
	void* buffer;
	uint16_t tamanio_string; // Uso esta variable para almacenar los tamanios de los string que vaya a ir recibiendo

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(uint16_t));
	recv(cliente, buffer, sizeof(uint16_t), 0);
	tamanio_string = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(cliente, buffer,tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	puts("Listo, recibi el paquete completo!\n");

	return paquete;
}

void enviar_select(int cliente, struct_select paquete){
	const uint8_t cod_op = SELECT;

	// Armo paquete con los datos
	size_t tamanio_paquete = sizeof(cod_op) + sizeof(uint16_t) + sizeof(paquete.key) + paquete.tamanio_nombre; // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero codigo de operacion
	memcpy(buffer, &cod_op, sizeof(cod_op)); // En el comienzo del buffer copio el codigo de operacion
	desplazamiento += sizeof(cod_op); // Me corro 1 byte del uint8_t

	// Ahora el nombre de la tabla
	memcpy(buffer + desplazamiento, &paquete.tamanio_nombre, sizeof(paquete.tamanio_nombre)); // En el comienzo del buffer copio el tamanio del nombre de la tabla
	desplazamiento += sizeof(paquete.tamanio_nombre); // Me corro 2 bytes del uint16
	memcpy(buffer + desplazamiento, paquete.nombreTabla, paquete.tamanio_nombre); // En la nueva posicion copio el nombre de la tabla
	desplazamiento += paquete.tamanio_nombre; // Me corro la longitud del string

	// Lo mismo para la clave
	memcpy(buffer + desplazamiento, &paquete.key, sizeof(paquete.key));
	// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer

	// Por ultimo envio el paquete y libero el buffer.
	send(cliente, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

void enviar_insert(int cliente, struct_insert paquete){
	const uint8_t cod_op = INSERT;

	// Armo paquete con los datos
	size_t tamanio_paquete = sizeof(cod_op) + sizeof(uint16_t)*2 + sizeof(paquete.key) + paquete.tamanio_nombre + paquete.tamanio_valor; // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero codigo de operacion
	memcpy(buffer, &cod_op, sizeof(cod_op)); // En el comienzo del buffer copio el codigo de operacion
	desplazamiento += sizeof(cod_op); // Me corro 1 byte del uint8_t

	// Ahora el nombre de la tabla
	memcpy(buffer + desplazamiento, &paquete.tamanio_nombre, sizeof(paquete.tamanio_nombre)); // En el comienzo del buffer copio el tamanio del nombre de la tabla
	desplazamiento += sizeof(paquete.tamanio_nombre); // Me corro 2 bytes del uint16
	memcpy(buffer + desplazamiento, paquete.nombreTabla, paquete.tamanio_nombre); // En la nueva posicion copio el nombre de la tabla
	desplazamiento += paquete.tamanio_nombre; // Me corro la longitud del string

	// Lo mismo para la clave
	memcpy(buffer + desplazamiento, &paquete.key, sizeof(paquete.key));
	desplazamiento += sizeof(paquete.key);

	// Lo mismo para el valor
	memcpy(buffer + desplazamiento, &paquete.tamanio_valor, sizeof(paquete.tamanio_valor));
	desplazamiento += sizeof(paquete.tamanio_valor);
	memcpy(buffer + desplazamiento, paquete.valor, paquete.tamanio_valor);
	// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer

	// Por ultimo envio el paquete y libero el buffer.
	send(cliente, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

void enviar_describe(int cliente, struct_describe paquete){
	const uint8_t cod_op = DESCRIBE;

	uint16_t tamanio_nombre = strlen(paquete.nombreTabla)+1; // Calculo el tamanio del nombre

	// Armo paquete con los datos
	size_t tamanio_paquete = sizeof(cod_op) + sizeof(uint16_t) + tamanio_nombre; // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero codigo de operacion
	memcpy(buffer, &cod_op, sizeof(cod_op)); // En el comienzo del buffer copio el codigo de operacion
	desplazamiento += sizeof(cod_op); // Me corro 1 byte del uint8_t

	// Ahora el nombre de la tabla
	memcpy(buffer + desplazamiento, &tamanio_nombre, sizeof(tamanio_nombre)); // En el comienzo del buffer copio el tamanio del nombre de la tabla
	desplazamiento += sizeof(tamanio_nombre); // Me corro 2 bytes del uint16
	memcpy(buffer + desplazamiento, paquete.nombreTabla, tamanio_nombre); // En la nueva posicion copio el nombre de la tabla
	// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer

	// Por ultimo envio el paquete y libero el buffer.
	send(cliente, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

void cliente(){
	log_trace(logger, "Iniciando cliente");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr("127.0.0.1"); // Direccion IP
	direccionServidor.sin_port = htons(8080); // PUERTO

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	log_trace(logger, "Conectando con servidor (FS)");
	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor))) {
		log_trace(logger, "No se pudo conectar con el servidor (FS)");
		return; // No seria la manera mas prolija de atender esto. Habria que seguir intentando
	}

	/*
	 * TODO:
	 * Hago handshake
	 * Enviar datos
	 */

	close(cliente); // No me olvido de cerrar el socket que ya no voy a usar
}

void servidor() {
	log_trace(logger, "Iniciando servidor");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(8080); // Puerto

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor))) {
		log_trace(logger, "Fallo el servidor");
		exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
	}

	listen(servidor, SOMAXCONN);
	log_trace(logger, "Escuchando");

	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion = sizeof(direccionCliente);
	int cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
	printf("Recibi una conexion en %d\n", cliente);

	/*
	 * TODO:
	 * Hago handshake
	 */

	while(1){
		// Recibo el codigo de op
		uint8_t cod_op;
		if(!recv(cliente, &cod_op, sizeof(uint8_t), 0)){ // Problema, recv es no bloqueante, asi que estoy en espera activa hasta
													  	 // que se desconecte el cliente o reciba algo. Deberiamos usar select()?
			log_trace(logger, "El cliente se desconecto");
			break;
		}

		switch (cod_op) {
			case SELECT:
			{
				log_trace(logger, "Recibi un SELECT");
				struct_select paquete = recibir_select(cliente);

				/*
				 * Depues haria lo que tenga que hacer con esta struct ya cargada
				 */
				printf("Comando recibido: SELECT %s %s\n\n", paquete.nombreTabla, paquete.key);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
				free(paquete.key);
			}
			break;
			case INSERT:
			{
				log_trace(logger, "Recibi un INSERT");
				struct_insert paquete = recibir_insert(cliente);

				/*
				 * Depues haria lo que tenga que hacer con esta struct ya cargada
				 */
				printf("Comando recibido: INSERT %s %s \"%s\"\n\n", paquete.nombreTabla, paquete.key, paquete.valor);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
				free(paquete.key);
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
			case JOURNAL:
				break;
			default:
				log_trace(logger, "Recibi una operacion invalida...");
		}
	}
	close(cliente); // No me olvido de cerrar el socket que ya no voy a usar mas
	close(servidor); // No me olvido de cerrar el socket que ya no voy a usar mas
}
