#include "serializacion.h"

struct_select recibir_select(int socket){
	struct_select paquete;
	void* buffer;
	uint16_t tamanio_string; // Uso esta variable para almacenar los tamanios de los string que vaya a ir recibiendo

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(uint16_t));
	recv(socket, buffer, sizeof(uint16_t), 0);
	tamanio_string = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);


	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	//printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la key
	buffer = malloc(sizeof(paquete.key));
	recv(socket, buffer, sizeof(paquete.key), 0);
	paquete.key = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("La key es %d\n", paquete.key);
	free(buffer);

	//puts("Listo, recibi el paquete completo!\n");

	return paquete;
	// Para tener en cuenta: El recv recibe un maximo de bytes como diga el tercer parametro,
	// pero puede recibir menos por alguna circunstancia no esperada (si llegaron menos bytes de los que esperaba).
	// La cantidad de bytes recibidos es el valor que devuelve recv, por lo que podemos almacenar y usar ese valor
	// para controlar posibles errores y abortar la operacion.
}

struct_insert recibir_insert(int socket){
	struct_insert paquete;
	void* buffer = NULL;
	uint16_t tamanio_string; // Uso esta variable para almacenar los tamanios de los string que vaya a ir recibiendo

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(tamanio_string));
	recv(socket, buffer, sizeof(tamanio_string), 0);
	tamanio_string = *((uint16_t*)buffer);
	//printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	//printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la key
	buffer = malloc(sizeof(paquete.key));
	recv(socket, buffer, sizeof(uint16_t), 0);
	paquete.key = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("La key es %d\n", paquete.key);
	free(buffer);

	// Por ultimo el valor
	buffer = malloc(sizeof(uint16_t));
	recv(socket, buffer, sizeof(uint16_t), 0);
	tamanio_string = *((uint16_t*)buffer);
	//printf("El valor es de %d bytes\n", tamanio_string);
	free(buffer);

	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.valor = malloc(tamanio_string);
	memcpy(paquete.valor, buffer, tamanio_string);
	//printf("El valor es \"%s\"\n", paquete.valor);
	free(buffer);

	//puts("Listo, recibi el paquete completo!\n");

	return paquete;
}

struct_insert recibir_insert_ts(int socket){
	struct_insert paquete;
	void* buffer = NULL;
	uint16_t tamanio_string; // Uso esta variable para almacenar los tamanios de los string que vaya a ir recibiendo

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(tamanio_string));
	recv(socket, buffer, sizeof(tamanio_string), 0);
	tamanio_string = *((uint16_t*)buffer);
	//printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	//printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la key
	buffer = malloc(sizeof(paquete.key));
	recv(socket, buffer, sizeof(uint16_t), 0);
	paquete.key = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("La key es %d\n", paquete.key);
	free(buffer);

	// Ahora el valor
	buffer = malloc(sizeof(uint16_t));
	recv(socket, buffer, sizeof(uint16_t), 0);
	tamanio_string = *((uint16_t*)buffer);
	//printf("El valor es de %d bytes\n", tamanio_string);
	free(buffer);

	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.valor = malloc(tamanio_string);
	memcpy(paquete.valor, buffer, tamanio_string);
	//printf("El valor es \"%s\"\n", paquete.valor);
	free(buffer);

	// Por ultimo el timestamp
	buffer = malloc(sizeof(paquete.timestamp));
	recv(socket, buffer, sizeof(uint64_t), 0);
	paquete.timestamp = *((uint64_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("El timestamp es %lld\n", paquete.timestamp);
	free(buffer);


	//puts("Listo, recibi el paquete completo!\n");

	return paquete;
}

struct_create recibir_create(int socket){
	struct_create paquete;
	void* buffer;
	uint16_t tamanio_string; // Uso esta variable para almacenar los tamanios de los string que vaya a ir recibiendo

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(uint16_t));
	recv(socket, buffer, sizeof(uint16_t), 0);
	tamanio_string = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	//printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la consistencia
	buffer = malloc(sizeof(paquete.consistencia));
	recv(socket, buffer, sizeof(paquete.consistencia), 0);
	paquete.consistencia = *((uint8_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("La consistencia es %d\n", paquete.consistencia);
	free(buffer);

	// Luego las particiones
	buffer = malloc(sizeof(paquete.particiones));
	recv(socket, buffer, sizeof(paquete.particiones), 0);
	paquete.particiones = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("Las particiones son %d\n", paquete.particiones);
	free(buffer);

	// Por ultimo el tiempo de compactacion
	buffer = malloc(sizeof(paquete.tiempoCompactacion));
	recv(socket, buffer, sizeof(paquete.tiempoCompactacion), 0);
	paquete.tiempoCompactacion = *((uint32_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("El tiempo de compactacion es %d\n", paquete.tiempoCompactacion);
	free(buffer);

	//puts("Listo, recibi el paquete completo!\n");

	return paquete;
	// Para tener en cuenta: El recv recibe un maximo de bytes como diga el tercer parametro,
	// pero puede recibir menos por alguna circunstancia no esperada (si llegaron menos bytes de los que esperaba).
	// La cantidad de bytes recibidos es el valor que devuelve recv, por lo que podemos almacenar y usar ese valor
	// para controlar posibles errores y abortar la operacion.
}

struct_describe recibir_describe(int socket){
	struct_describe paquete;
	void* buffer;
	uint16_t tamanio_string; // Uso esta variable para almacenar los tamanios de los string que vaya a ir recibiendo

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(uint16_t));
	recv(socket, buffer, sizeof(uint16_t), 0);
	tamanio_string = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer,tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	//printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	//puts("Listo, recibi el paquete completo!\n");

	return paquete;
}

struct_describe recibir_drop(int socket){
	return recibir_describe(socket); // Es la misma funcion, jaja
}

void enviar_select(int socket, struct_select paquete){
	const uint8_t cod_op = SELECT;

	uint16_t tamanio_nombre = strlen(paquete.nombreTabla) + 1; // Calculo el tamanio del nombre

	// Armo paquete con los datos
	size_t tamanio_paquete = sizeof(cod_op) + sizeof(uint16_t) + sizeof(paquete.key) + tamanio_nombre; // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero codigo de operacion
	memcpy(buffer, &cod_op, sizeof(cod_op)); // En el comienzo del buffer copio el codigo de operacion
	desplazamiento += sizeof(cod_op); // Me corro 1 byte del uint8_t

	// Ahora el nombre de la tabla
	memcpy(buffer + desplazamiento, &tamanio_nombre, sizeof(tamanio_nombre)); // En el comienzo del buffer copio el tamanio del nombre de la tabla
	desplazamiento += sizeof(tamanio_nombre); // Me corro 2 bytes del uint16
	memcpy(buffer + desplazamiento, paquete.nombreTabla, tamanio_nombre); // En la nueva posicion copio el nombre de la tabla
	desplazamiento += tamanio_nombre; // Me corro la longitud del string

	// Lo mismo para la clave
	memcpy(buffer + desplazamiento, &paquete.key, sizeof(paquete.key));
	// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer

	// Por ultimo envio el paquete y libero el buffer.
	send(socket, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

void enviar_insert(int socket, struct_insert paquete){
	const uint8_t cod_op = INSERT;

	uint16_t tamanio_nombre = strlen(paquete.nombreTabla) + 1; // Calculo el tamanio del nombre
	uint16_t tamanio_valor = strlen(paquete.valor) + 1; // Calculo el tamanio del valor

	// Armo paquete con los datos
	size_t tamanio_paquete = sizeof(cod_op) + sizeof(uint16_t)*2 + sizeof(paquete.key) + tamanio_nombre + tamanio_valor; // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero codigo de operacion
	memcpy(buffer, &cod_op, sizeof(cod_op)); // En el comienzo del buffer copio el codigo de operacion
	desplazamiento += sizeof(cod_op); // Me corro 1 byte del uint8_t

	// Ahora el nombre de la tabla
	memcpy(buffer + desplazamiento, &tamanio_nombre, sizeof(tamanio_nombre)); // En el comienzo del buffer copio el tamanio del nombre de la tabla
	desplazamiento += sizeof(tamanio_nombre); // Me corro 2 bytes del uint16
	memcpy(buffer + desplazamiento, paquete.nombreTabla, tamanio_nombre); // En la nueva posicion copio el nombre de la tabla
	desplazamiento += tamanio_nombre; // Me corro la longitud del string

	// Lo mismo para la clave
	memcpy(buffer + desplazamiento, &paquete.key, sizeof(paquete.key));
	desplazamiento += sizeof(paquete.key);

	// Lo mismo para el valor
	memcpy(buffer + desplazamiento, &tamanio_valor, sizeof(tamanio_valor));
	desplazamiento += sizeof(tamanio_valor);
	memcpy(buffer + desplazamiento, paquete.valor, tamanio_valor);
	// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer

	// Por ultimo envio el paquete y libero el buffer.
	send(socket, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

// Funcion para enviar un insert con timestamp
void enviar_insert_ts(int socket, struct_insert paquete){
	const uint8_t cod_op = INSERT;

	uint16_t tamanio_nombre = strlen(paquete.nombreTabla) + 1; // Calculo el tamanio del nombre
	uint16_t tamanio_valor = strlen(paquete.valor) + 1; // Calculo el tamanio del valor

	// Armo paquete con los datos
	size_t tamanio_paquete = sizeof(cod_op) + sizeof(uint16_t)*2 + sizeof(paquete.key) + sizeof(paquete.timestamp) + tamanio_nombre + tamanio_valor; // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero codigo de operacion
	memcpy(buffer, &cod_op, sizeof(cod_op)); // En el comienzo del buffer copio el codigo de operacion
	desplazamiento += sizeof(cod_op); // Me corro 1 byte del uint8_t

	// Ahora el nombre de la tabla
	memcpy(buffer + desplazamiento, &tamanio_nombre, sizeof(tamanio_nombre)); // En el comienzo del buffer copio el tamanio del nombre de la tabla
	desplazamiento += sizeof(tamanio_nombre); // Me corro 2 bytes del uint16
	memcpy(buffer + desplazamiento, paquete.nombreTabla, tamanio_nombre); // En la nueva posicion copio el nombre de la tabla
	desplazamiento += tamanio_nombre; // Me corro la longitud del string

	// Lo mismo para la clave
	memcpy(buffer + desplazamiento, &paquete.key, sizeof(paquete.key));
	desplazamiento += sizeof(paquete.key);

	// Lo mismo para el valor
	memcpy(buffer + desplazamiento, &tamanio_valor, sizeof(tamanio_valor));
	desplazamiento += sizeof(tamanio_valor);
	memcpy(buffer + desplazamiento, paquete.valor, tamanio_valor);
	desplazamiento += tamanio_valor;

	// Por ultimo el timestamp
	memcpy(buffer + desplazamiento, &paquete.timestamp, sizeof(paquete.timestamp));
	// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer

	// Por ultimo envio el paquete y libero el buffer.
	send(socket, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

void enviar_create(int socket, struct_create paquete){
	const uint8_t cod_op = CREATE;

	uint16_t tamanio_nombre = strlen(paquete.nombreTabla) + 1; // Calculo el tamanio del nombre

	// Armo paquete con los datos
	size_t tamanio_paquete = sizeof(cod_op) + sizeof(uint16_t) + sizeof(paquete.consistencia) + sizeof(paquete.particiones) + sizeof(paquete.tiempoCompactacion) + tamanio_nombre; // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero codigo de operacion
	memcpy(buffer, &cod_op, sizeof(cod_op)); // En el comienzo del buffer copio el codigo de operacion
	desplazamiento += sizeof(cod_op); // Me corro 1 byte del uint8_t

	// Ahora el nombre de la tabla
	memcpy(buffer + desplazamiento, &tamanio_nombre, sizeof(tamanio_nombre)); // En el comienzo del buffer copio el tamanio del nombre de la tabla
	desplazamiento += sizeof(tamanio_nombre); // Me corro 2 bytes del uint16
	memcpy(buffer + desplazamiento, paquete.nombreTabla, tamanio_nombre); // En la nueva posicion copio el nombre de la tabla
	desplazamiento += tamanio_nombre; // Me corro la longitud del string

	// Lo mismo para la consistencia
	memcpy(buffer + desplazamiento, &paquete.consistencia, sizeof(paquete.consistencia));
	desplazamiento += sizeof(paquete.consistencia); // Me corro 1 byte del uint8

	// Luego la cantidad de particiones
	memcpy(buffer + desplazamiento, &paquete.particiones, sizeof(paquete.particiones));
	desplazamiento += sizeof(paquete.particiones); // Me corro 2 byte del uint16

	// Por ultimo el tiempo de compactacion
	memcpy(buffer + desplazamiento, &paquete.tiempoCompactacion, sizeof(paquete.tiempoCompactacion));
	// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer

	// Por ultimo envio el paquete y libero el buffer.
	send(socket, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

void enviar_describe(int socket, struct_describe paquete){
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
	send(socket, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

void enviar_drop(int socket, struct_describe paquete){
	const uint8_t cod_op = DROP;

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
	send(socket, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

void enviar_journal(int socket){
	const uint8_t cod_op = JOURNAL;
	send(socket, &cod_op, sizeof(cod_op), 0);
}

void enviar_describe_global(int socket){
	const uint8_t cod_op = DESCRIBE_GLOBAL;
	send(socket, &cod_op, sizeof(cod_op), 0);
}

void enviar_registro(int socket, struct_select_respuesta registro){
	uint16_t tamanio_valor = strlen(registro.valor) + 1; // Calculo el tamanio del valor

	// Armo paquete con los datos
	size_t tamanio_paquete = sizeof(tamanio_valor) + sizeof(registro.estado) + sizeof(registro.timestamp) + tamanio_valor; // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero el estado de la peticion
	memcpy(buffer + desplazamiento, &registro.estado, sizeof(registro.estado));
	desplazamiento += sizeof(registro.estado);

	if(registro.estado == ESTADO_SELECT_OK){ // Si el estado no es OK, es al pedo mandar mas data.
		// Ahora el valor
		memcpy(buffer + desplazamiento, &tamanio_valor, sizeof(tamanio_valor));
		desplazamiento += sizeof(tamanio_valor);
		memcpy(buffer + desplazamiento, registro.valor, tamanio_valor);
		desplazamiento += tamanio_valor;

		// Por ultimo el timestamp
		memcpy(buffer + desplazamiento, &registro.timestamp, sizeof(registro.timestamp));
		// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer
	}
	else{
		tamanio_paquete = sizeof(registro.estado); // Evito mandar el buffer completo
	}

	// Por ultimo envio el paquete y libero el buffer.
	send(socket, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

struct_select_respuesta recibir_registro(int socket){
	struct_select_respuesta paquete;
	void* buffer = NULL;
	uint16_t tamanio_string; // Uso esta variable para almacenar los tamanios de los string que vaya a ir recibiendo

	// Primero recibo el estado
	buffer = malloc(sizeof(paquete.estado));
	recv(socket, buffer, sizeof(paquete.estado), 0);
	paquete.estado = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("El estado es %d\n", paquete.estado);
	free(buffer);

	if(paquete.estado == ESTADO_SELECT_OK){ // Si el estado no es OK, es al pedo el resto de data.
		// Ahora recibo el valor
		buffer = malloc(sizeof(uint16_t));
		recv(socket, buffer, sizeof(uint16_t), 0);
		tamanio_string = *((uint16_t*)buffer);
		//printf("El valor es de %d bytes\n", tamanio_string);
		free(buffer);

		buffer = malloc(tamanio_string);
		recv(socket, buffer, tamanio_string, 0);
		paquete.valor = malloc(tamanio_string);
		memcpy(paquete.valor, buffer, tamanio_string);
		//printf("El valor es \"%s\"\n", paquete.valor);
		free(buffer);

		// Por ultimo el timestamp
		buffer = malloc(sizeof(paquete.timestamp));
		recv(socket, buffer, sizeof(uint64_t), 0);
		paquete.timestamp = *((uint64_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
		//printf("El timestamp es %lld\n", paquete.timestamp);
		free(buffer);
	}

	//puts("Listo, recibi el paquete completo!\n");

	return paquete;
}

void enviar_estado(int socket, uint16_t estado){
	send(socket, &estado, sizeof(estado), 0);
}
uint16_t recibir_estado(int socket){
	uint16_t estado;
	recv(socket, &estado, sizeof(estado), 0);
	return estado;
}

void responder_create(int socket, enum estados_create estado){
	enviar_estado(socket, estado); // Simplemente mando el estado
}

enum estados_create recibir_respuesta_create(int socket){
	return recibir_estado(socket);
}

void responder_insert(int socket, enum estados_insert estado){
	enviar_estado(socket, estado); // Simplemente mando el estado
}

void enviar_respuesta_describe(int socket, struct_describe_respuesta respuesta){
	// Armo paquete con los datos
	size_t tamanio_paquete = sizeof(respuesta.estado) + sizeof(respuesta.consistencia) + sizeof(respuesta.particiones) + sizeof(respuesta.tiempo_compactacion); // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero el estado de la peticion
	memcpy(buffer + desplazamiento, &respuesta.estado, sizeof(respuesta.estado));
	desplazamiento += sizeof(respuesta.estado);

	if(respuesta.estado == ESTADO_DESCRIBE_OK){ // Si el estado no es OK, es al pedo mandar mas data.
		// Ahora la consistencia
		memcpy(buffer + desplazamiento, &respuesta.consistencia, sizeof(respuesta.consistencia));
		desplazamiento += sizeof(respuesta.consistencia);
		// Luego las particiones
		memcpy(buffer + desplazamiento, &respuesta.particiones, sizeof(respuesta.particiones));
		desplazamiento += sizeof(respuesta.particiones);

		// Por ultimo el tiempo de compactacion
		memcpy(buffer + desplazamiento, &respuesta.tiempo_compactacion, sizeof(respuesta.tiempo_compactacion));
		// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer
	}
	else{
		tamanio_paquete = sizeof(respuesta.estado); // Evito mandar el buffer completo
	}

	// Por ultimo envio el paquete y libero el buffer.
	send(socket, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
	free(buffer);
}

struct_describe_respuesta recibir_respuesta_describe(int socket){
	struct_describe_respuesta paquete;
	void* buffer = NULL;

	// Primero recibo el estado
	buffer = malloc(sizeof(paquete.estado));
	recv(socket, buffer, sizeof(paquete.estado), 0);
	paquete.estado = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("El estado es %d\n", paquete.estado);
	free(buffer);

	if(paquete.estado == ESTADO_DESCRIBE_OK){ // Si el estado no es OK, es al pedo el resto de data.
		// Ahora recibo la consistencia
		buffer = malloc(sizeof(uint16_t));
		recv(socket, buffer, sizeof(uint16_t), 0);
		paquete.consistencia = *((uint16_t*)buffer);
		//printf("La consistencia es %d\n", paquete.consistencia);
		free(buffer);

		// Luego las particiones
		buffer = malloc(sizeof(uint16_t));
		recv(socket, buffer, sizeof(uint16_t), 0);
		paquete.particiones = *((uint16_t*)buffer);
		//printf("Las particiones son %d\n", paquete.particiones);
		free(buffer);

		// Por ultimo el tiempo de compactacion
		buffer = malloc(sizeof(paquete.tiempo_compactacion));
		recv(socket, buffer, sizeof(uint32_t), 0);
		paquete.tiempo_compactacion = *((uint32_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
		//printf("El tiempo de compactacion es %d\n", paquete.tiempo_compactacion);
		free(buffer);
	}

	//puts("Listo, recibi el paquete completo!\n");

	return paquete;
}

void enviar_respuesta_describe_global(int socket, struct_describe_global_respuesta respuesta){
	// Vamos a enviar un paquete inicial con estado y cantidad de tablas descriptas
	uint32_t cantidad_describes = dictionary_size(respuesta.describes);
	size_t tamanio_paquete = sizeof(respuesta.estado) + sizeof(cantidad_describes);
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero el estado de la peticion
	memcpy(buffer + desplazamiento, &respuesta.estado, sizeof(respuesta.estado));
	desplazamiento += sizeof(respuesta.estado);

	if(respuesta.estado == ESTADO_DESCRIBE_OK){ // Si el estado no es OK, es al pedo mandar mas data.
		// Ahora la cantidad de describes
		memcpy(buffer + desplazamiento, &cantidad_describes, sizeof(cantidad_describes));
		// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer
	}
	else{
		tamanio_paquete = sizeof(respuesta.estado); // Evito mandar el buffer completo
	}

	// Luego envio el paquete y libero el buffer.
	send(socket, buffer, tamanio_paquete, 0);
	free(buffer);

	// Por ultimo enviamos una respuesta_describe por cada tabla descripta
	if(respuesta.estado == ESTADO_DESCRIBE_OK){
		void enviar_describe_individual(char* nombre_tabla, struct_describe_respuesta* describe){
			// Enviamos el nombre de la tabla
			uint16_t tamanio_nombre = strlen(nombre_tabla)+1; // Calculo el tamanio del nombre
			tamanio_paquete = sizeof(tamanio_nombre) + tamanio_nombre; // Evito mandar el buffer completo
			buffer = malloc(tamanio_paquete);
			memcpy(buffer, &tamanio_nombre, sizeof(tamanio_nombre)); // En el comienzo del buffer copio el tamanio del nombre de la tabla
			desplazamiento = sizeof(tamanio_nombre); // Me corro 2 bytes del uint16
			memcpy(buffer + desplazamiento, nombre_tabla, tamanio_nombre); // En la nueva posicion copio el nombre de la tabla
			// Al pedo calcular el desplazamiento ahora, no voy a enviar mas nada y ademas ya me ocupe todo el buffer

			// Ahora envio el paquete y libero el buffer.
			send(socket, buffer, tamanio_paquete, 0); // Hago un solo send para todo, asi nos aseguramos que el paquete llega en orden
			free(buffer);

			// Por ultimo mandamos la metadata
			enviar_respuesta_describe(socket, *describe);
		}
		dictionary_iterator(respuesta.describes, (void(*)(char*,void*))enviar_describe_individual);
	}
}

struct_describe_global_respuesta recibir_respuesta_describe_global(int socket){
	struct_describe_global_respuesta paquete;
	paquete.describes = dictionary_create();
	void* buffer = NULL;

	// Primero recibo el estado
	buffer = malloc(sizeof(paquete.estado));
	recv(socket, buffer, sizeof(paquete.estado), 0);
	paquete.estado = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	//printf("El estado es %d\n", paquete.estado);
	free(buffer);

	if(paquete.estado == ESTADO_DESCRIBE_OK){ // Si el estado no es OK, es al pedo el resto de data.
		// Ahora recibo la cantidad de tablas descriptas
		buffer = malloc(sizeof(uint32_t));
		recv(socket, buffer, sizeof(uint32_t), 0);
		uint32_t cantidad_describes = *((uint32_t*)buffer);
		//printf("La cantidad de describes es %d\n", cantidad_describes);
		free(buffer);

		// Ahora recibo todos los describes
		for(int i = 0; i < cantidad_describes; i++){
			// Primero nombre de tabla
			char* nombre_tabla = recibir_describe(socket).nombreTabla;

			struct_describe_respuesta* describe = malloc(sizeof(struct_describe_respuesta));
			*describe = recibir_respuesta_describe(socket);
			dictionary_put(paquete.describes, nombre_tabla, describe);
		}
	}
	if(paquete.estado > ESTADO_DESCRIBE_ERROR_OTRO){
		paquete.estado = ESTADO_DESCRIBE_ERROR_OTRO;
	}

	//puts("Listo, recibi el paquete completo!\n");

	return paquete;
}

enum estados_insert recibir_respuesta_insert(int socket){
	return recibir_estado(socket);
}

void responder_drop(int socket, enum estados_drop estado){
	enviar_estado(socket, estado); // Simplemente mando el estado
}

enum estados_drop recibir_respuesta_drop(int socket){
	return recibir_estado(socket);
}

void responder_journal(int socket, enum estados_journal estado){
	enviar_estado(socket, estado); // Simplemente mando el estado
}

enum estados_journal recibir_respuesta_journal(int socket){
	return recibir_estado(socket);
}

void enviar_tabla_gossiping(int socket, t_list* tabla){
	uint16_t cantidad_memorias = list_size(tabla);

	size_t tamanio_memoria = sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t);
	size_t tamanio_paquete = sizeof(cantidad_memorias) + (tamanio_memoria * cantidad_memorias); // Calculo el tamanio del paquete
	void* buffer = malloc(tamanio_paquete); // Pido memoria para el tamanio del paquete completo que voy a enviar

	int desplazamiento = 0; // Voy a usar esta variable para ir moviendome por el buffer

	// Primero la cantidad
	memcpy(buffer, &cantidad_memorias, sizeof(cantidad_memorias));
	desplazamiento += sizeof(cantidad_memorias);

	void enviar_memoria(t_memoria* memoria){
		// Primero numero de Memoria
		memcpy(buffer + desplazamiento, &memoria->numero, sizeof(memoria->numero));
		desplazamiento += sizeof(memoria->numero);

		// Ahora la IP
		uint32_t IP = inet_addr(memoria->IP);
		memcpy(buffer + desplazamiento, &IP, sizeof(IP));
		desplazamiento += sizeof(IP);

		// Por ultimo el puerto
		memcpy(buffer + desplazamiento, &memoria->puerto, sizeof(memoria->puerto));
		desplazamiento += sizeof(memoria->puerto);
	}
	list_iterate(tabla, (void(*)(void*)) enviar_memoria);

	send(socket, buffer, tamanio_paquete, 0);
}

t_list* recibir_tabla_gossiping(int socket){
	// Primero recibimos la cantidad
	uint16_t cantidad_memorias;
	recv(socket, &cantidad_memorias, sizeof(cantidad_memorias), 0);

	t_list* tabla = list_create();
	for (int i = 0; i < cantidad_memorias; ++i) {
		t_memoria* memoria = malloc(sizeof(t_memoria));

		// Primero recibimos el numero de memoria
		recv(socket, &(memoria->numero), sizeof(memoria->numero), 0);

		// Ahora la IP
		struct in_addr addr;
		recv(socket, &addr.s_addr, sizeof(uint32_t), 0);
		memoria->IP = strdup(inet_ntoa(addr));

		// Chanchada para obtener la IP de la memoria a la que nos conectamos y que no sabe su propia IP
		if(!strcmp(memoria->IP, "0.0.0.0")){
			struct sockaddr_in addr;
			socklen_t addr_size = sizeof(struct sockaddr_in);
			getpeername(socket, (struct sockaddr *)&addr, &addr_size);
			memoria->IP = strdup(inet_ntoa(addr.sin_addr));
		}
		//printf("IP del otro %s", memoria->IP);

		// Por ultimo el puerto
		recv(socket, &(memoria->puerto), sizeof(memoria->puerto), 0);

		list_add(tabla, memoria);
	}

	return tabla;
}
