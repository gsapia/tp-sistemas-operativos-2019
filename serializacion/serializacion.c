#include "serializacion.h"

struct_select recibir_select(int socket){
	struct_select paquete;
	void* buffer;
	uint16_t tamanio_string; // Uso esta variable para almacenar los tamanios de los string que vaya a ir recibiendo

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(uint16_t));
	recv(socket, buffer, sizeof(uint16_t), 0);
	tamanio_string = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la key
	buffer = malloc(sizeof(paquete.key));
	recv(socket, buffer, sizeof(paquete.key), 0);
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

struct_insert recibir_insert(int socket){
	struct_insert paquete;
	void* buffer = NULL;
	uint16_t tamanio_string; // Uso esta variable para almacenar los tamanios de los string que vaya a ir recibiendo

	// Recibo tamanio del nombre de tabla
	buffer = malloc(sizeof(tamanio_string));
	recv(socket, buffer, sizeof(tamanio_string), 0);
	tamanio_string = *((uint16_t*)buffer);
	printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la key
	buffer = malloc(sizeof(paquete.key));
	recv(socket, buffer, sizeof(uint16_t), 0);
	paquete.key = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("La key es %d\n", paquete.key);
	free(buffer);

	// Por ultimo el valor
	buffer = malloc(sizeof(uint16_t));
	recv(socket, buffer, sizeof(uint16_t), 0);
	tamanio_string = *((uint16_t*)buffer);
	printf("El valor es de %d bytes\n", tamanio_string);
	free(buffer);

	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.valor = malloc(tamanio_string);
	memcpy(paquete.valor, buffer, tamanio_string);
	printf("El valor es \"%s\"\n", paquete.valor);
	free(buffer);

	puts("Listo, recibi el paquete completo!\n");

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
	printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la key
	buffer = malloc(sizeof(paquete.key));
	recv(socket, buffer, sizeof(uint16_t), 0);
	paquete.key = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("La key es %d\n", paquete.key);
	free(buffer);

	// Ahora el valor
	buffer = malloc(sizeof(uint16_t));
	recv(socket, buffer, sizeof(uint16_t), 0);
	tamanio_string = *((uint16_t*)buffer);
	printf("El valor es de %d bytes\n", tamanio_string);
	free(buffer);

	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.valor = malloc(tamanio_string);
	memcpy(paquete.valor, buffer, tamanio_string);
	printf("El valor es \"%s\"\n", paquete.valor);
	free(buffer);

	// Por ultimo el timestamp
	buffer = malloc(sizeof(paquete.timestamp));
	recv(socket, buffer, sizeof(uint64_t), 0);
	paquete.timestamp = *((uint64_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("El timestamp es %lld\n", paquete.timestamp);
	free(buffer);


	puts("Listo, recibi el paquete completo!\n");

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
	printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer, tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	// Ahora recibo la consistencia
	buffer = malloc(sizeof(paquete.consistencia));
	recv(socket, buffer, sizeof(paquete.consistencia), 0);
	paquete.consistencia = *((uint8_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("La consistencia es %d\n", paquete.consistencia);
	free(buffer);

	// Luego las particiones
	buffer = malloc(sizeof(paquete.particiones));
	recv(socket, buffer, sizeof(paquete.particiones), 0);
	paquete.particiones = *((uint16_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("Las particiones son %d\n", paquete.particiones);
	free(buffer);

	// Por ultimo el tiempo de compactacion
	buffer = malloc(sizeof(paquete.tiempoCompactacion));
	recv(socket, buffer, sizeof(paquete.tiempoCompactacion), 0);
	paquete.tiempoCompactacion = *((uint32_t*)buffer); // Casteo el puntero a void a un puntero a uint para despues buscar el valor al que apunta
	printf("El tiempo de compactacion es %d\n", paquete.tiempoCompactacion);
	free(buffer);

	puts("Listo, recibi el paquete completo!\n");

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
	printf("El nombre de tabla es de %d bytes\n", tamanio_string);
	free(buffer);

	// Ahora recibo el nombre de la tabla
	buffer = malloc(tamanio_string);
	recv(socket, buffer,tamanio_string, 0);
	paquete.nombreTabla = malloc(tamanio_string);
	memcpy(paquete.nombreTabla, buffer, tamanio_string);
	printf("El nombre de tabla es %s\n", paquete.nombreTabla);
	free(buffer);

	puts("Listo, recibi el paquete completo!\n");

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
