#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "serializacion.h"

int main(void) {
	puts("Iniciando cliente");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr("127.0.0.1"); // Direccion IP
	direccionServidor.sin_port = htons(8080); // PUERTO

	int cliente = socket(AF_INET, SOCK_STREAM, 0);

	puts("Conectando con servidor (FS)");
	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor))) {
		puts("No se pudo conectar con el servidor (FS)");
		exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
	}

	// Armo un INSERT TABLA1 KEY1 "ALGUN VALOR"
	struct_insert paquete;
	paquete.nombreTabla = "TABLA1"; // 7 bytes
	paquete.key = 1; // 2 bytes
	paquete.valor = "ALGUN VALOR"; // 12 bytes
	paquete.timestamp = 1558109385595; // No es necesario si no enviamos timestamp

	// Lo envio
	puts("Enviando INSERT");
	enviar_insert_ts(cliente, paquete); // Usar enviar_insert si no hace falta el timestamp (Kernel)
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Ahora me quedo esperando la respuesta
	enum estados_insert estado3 = recibir_respuesta_insert(cliente);
	switch (estado3) {
	case ESTADO_INSERT_OK:
		printf("Registro agregado\n");
		break;
	case ESTADO_INSERT_ERROR_TABLA:
		printf("ERROR: no existe esa tabla");
		break;
		// Paja hacer los demas case
	}

	// Armo un SELECT TABLA1 5
	struct_select paquete2;
	paquete2.nombreTabla = "TABLA1";
	paquete2.key = 5;

	// Lo envio
	puts("Enviando SELECT TABLA1 5");
	enviar_select(cliente, paquete2);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Ahora me quedo esperando la respuesta
	struct_select_respuesta registro = recibir_registro(cliente);
	switch (registro.estado) {
		case ESTADO_SELECT_OK:
			printf("Respuesta SELECT: Valor:%s Timestamp:%lld\n",registro.valor, registro.timestamp);
			break;
		case ESTADO_SELECT_ERROR_KEY:
			printf("Respuesta SELECT: ERROR No existe un registro con esa clave\n");
			break;
		// Paja hacer los demas case
	}

	// Armo un SELECT TABLA1 2 (Deberia fallar)
	struct_select paquete2_fail;
	paquete2_fail.nombreTabla = "TABLA1";
	paquete2_fail.key = 2;

	// Lo envio
	puts("Enviando SELECT TABLA1 2");
	enviar_select(cliente, paquete2_fail);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Ahora me quedo esperando la respuesta
	struct_select_respuesta registro2 = recibir_registro(cliente);
	switch (registro2.estado) {
		case ESTADO_SELECT_OK:
			printf("Respuesta SELECT: Valor:%s Timestamp:%lld\n",registro2.valor, registro2.timestamp);
			break;
		case ESTADO_SELECT_ERROR_KEY:
			printf("Respuesta SELECT: ERROR No existe un registro con esa clave\n");
			break;
		// Paja hacer los demas case
	}

	// Armo un CREATE TABLA1 SC 4
	struct_create paquete3;
	paquete3.nombreTabla = "TABLA1";
	paquete3.consistencia = SC;
	paquete3.particiones = 4;
	paquete3.tiempoCompactacion = 60000;

	// Lo envio
	puts("Enviando CREATE");
	enviar_create(cliente, paquete3);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Ahore me quedo esperando la respuesta
	uint16_t estado = recibir_respuesta_create(cliente);
	switch (estado) {
		case ESTADO_CREATE_OK:
			printf("La tabla se creo\n");
			break;
		case ESTADO_CREATE_ERROR_TABLAEXISTENTE:
			printf("La tabla ya existia\n");
			break;
		// Paja hacer los demas case
	}

	// Armo un CREATE TABLA2 SC 4 (Deberia fallar)
	struct_create paquete3_fail;
	paquete3_fail.nombreTabla = "TABLA2";
	paquete3_fail.consistencia = SC;
	paquete3_fail.particiones = 4;
	paquete3_fail.tiempoCompactacion = 60000;

	// Lo envio
	puts("Enviando CREATE");
	enviar_create(cliente, paquete3_fail);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Ahora me quedo esperando la respuesta
	uint16_t estado2 = recibir_respuesta_create(cliente);
	switch (estado2) {
	case ESTADO_CREATE_OK:
		printf("La tabla se creo\n");
		break;
	case ESTADO_CREATE_ERROR_TABLAEXISTENTE:
		printf("ERROR: La tabla ya existia\n");
		break;
		// Paja hacer los demas case
	}


	// Armo un DESCRIBE TABLA1
	struct_describe paquete4;
	paquete4.nombreTabla = "TABLA1";

	// Lo envio
	puts("Enviando DESCRIBE");
	enviar_describe(cliente, paquete4);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Ahora me quedo esperando la respuesta
	struct_describe_respuesta coso = recibir_respuesta_describe(cliente);
	switch (coso.estado) {
	case ESTADO_DESCRIBE_OK:
		printf("Describe OK\n");
		break;
	case ESTADO_DESCRIBE_ERROR_TABLA:
		printf("ERROR: La tabla no existe\n");
		break;
		// Paja hacer los demas case
	}

	// Armo un DROP TABLA1
	struct_drop paquete5;
	paquete5.nombreTabla = "TABLA1";

	// Lo envio
	puts("Enviando DROP");
	enviar_drop(cliente, paquete5);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Ahora me quedo esperando la respuesta
	enum estados_drop estado_drop = recibir_respuesta_drop(cliente);
	switch (estado_drop) {
		case ESTADO_DROP_OK:
			puts("DROP OK");
			break;
		case ESTADO_DROP_ERROR_TABLA:
			puts("ERROR: La tabla no existe");
			break;
			// Paja hacer los demas case
	}

	// Por ultimo envio un JOURNAL
	puts("Enviando JOURNAL");
	enviar_journal(cliente);

	// Ahora me quedo esperando la respuesta
	enum estados_journal estado_journal = recibir_respuesta_journal(cliente);
	switch (estado_journal) {
	case ESTADO_JOURNAL_OK:
		puts("JOURNAL OK");
		break;
	default:
		puts("ERROR: Ocurrio un error desconocido");
		break;
	}

	// Enviamos un DESCRIBE GLOBAL
	puts("Enviando DESCRIBE GLOBAL");
	enviar_describe_global(cliente);

	// Ahora me quedo esperando la respuesta
	struct_describe_global_respuesta respuesta_describe_global = recibir_respuesta_describe_global(cliente);
	if(respuesta_describe_global.estado == ESTADO_DESCRIBE_OK){
		void mostrar_describe(char* nombre_tabla, struct_describe_respuesta* describe){
			switch (describe->estado) {
				case ESTADO_DESCRIBE_OK:
					printf("Describe OK, Tabla: %s, Consistencia: %d, Particiones: %d Compactacion: %d\n", nombre_tabla, describe->consistencia, describe->particiones, describe->tiempo_compactacion);
					break;
					// Paja hacer los demas case
			}
		}
		dictionary_iterator(respuesta_describe_global.describes, mostrar_describe);
	}


	puts("Termine");
	close(cliente); // No me olvido de cerrar el socket que ya no voy a usar
	return EXIT_SUCCESS;
}
