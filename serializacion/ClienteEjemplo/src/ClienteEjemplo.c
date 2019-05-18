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

	// Armo un SELECT TABLA1 KEY1
	struct_select paquete2;
	paquete2.nombreTabla = "TABLA1";
	paquete2.key = 1;

	// Lo envio
	puts("Enviando SELECT");
	enviar_select(cliente, paquete2);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Armo un CREATE SC 4
	struct_create paquete3;
	paquete3.nombreTabla = "TABLA1";
	paquete3.consistencia = SC;
	paquete3.particiones = 4;
	paquete3.tiempoCompactacion = 60000;

	// Lo envio
	puts("Enviando CREATE");
	enviar_create(cliente, paquete3);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla


	// Armo un DESCRIBE TABLA1
	struct_describe paquete4;
	paquete4.nombreTabla = "TABLA1";

	// Lo envio
	puts("Enviando DESCRIBE");
	enviar_describe(cliente, paquete4);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Armo un DROP TABLA1
	struct_drop paquete5;
	paquete5.nombreTabla = "TABLA1";

	// Lo envio
	puts("Enviando DROP");
	enviar_drop(cliente, paquete5);
	// En este caso uso todos string literals, asi que no uso memoria dinamica. Pero si lo hiciese, hay que liberarla

	// Por ultimo envio un JOURNAL
	puts("Enviando JOURNAL");
	enviar_journal(cliente);

	puts("Termine");
	close(cliente); // No me olvido de cerrar el socket que ya no voy a usar
	return EXIT_SUCCESS;
}
