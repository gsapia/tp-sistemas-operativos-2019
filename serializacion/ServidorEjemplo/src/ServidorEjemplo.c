#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "serializacion.h"

int main(void) {
	puts("Iniciando servidor");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(8080); // Puerto

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor))) {
		puts("Fallo el servidor");
		exit(EXIT_FAILURE); // No seria la manera mas prolija de atender esto
	}

	listen(servidor, SOMAXCONN);
	puts("Escuchando");

	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion = sizeof(direccionCliente);
	int cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
	printf("Recibi una conexion en %d\n", cliente);

	while(1){
		// Recibo el codigo de op
		uint8_t cod_op;
		if(!recv(cliente, &cod_op, sizeof(uint8_t), 0)){ // Problema, recv es no bloqueante, asi que estoy en espera activa hasta
													  	 // que se desconecte el cliente o reciba algo. Deberiamos usar select()?
			puts("El cliente se desconecto");
			break;
		}

		switch (cod_op) {
			case SELECT:
			{
				puts("Recibi un SELECT");
				struct_select paquete = recibir_select(cliente);

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
				puts("Recibi un INSERT");
				struct_insert paquete = recibir_insert_ts(cliente); // Usar recibir_insert si no hace falta el timestamp (Memoria)

				/*
				 * Depues haria lo que tenga que hacer con esta struct ya cargada
				 */
				printf("Comando recibido: INSERT %s %d \"%s\" %lld\n\n", paquete.nombreTabla, paquete.key, paquete.valor, paquete.timestamp);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
				free(paquete.valor);
			}
			break;
			case CREATE:
			{
				puts("Recibi un CREATE");
				struct_create paquete = recibir_create(cliente);

				/*
				 * Depues haria lo que tenga que hacer con esta struct ya cargada
				 */
				printf("Comando recibido: CREATE %s %d %d %d\n\n", paquete.nombreTabla, paquete.consistencia, paquete.particiones, paquete.tiempoCompactacion);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
			break;
			case DESCRIBE:
			{
				puts("Recibi un DESCRIBE");
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
			{
				puts("Recibi un DROP");
				struct_drop paquete = recibir_drop(cliente);

				/*
				 * Depues haria lo que tenga que hacer con esta struct ya cargada
				 */
				printf("Comando recibido: DROP %s\n\n", paquete.nombreTabla);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
			break;
			case JOURNAL:
			{
				puts("Recibi un JOURNAL");
				/*
				 * Depues haria lo que tenga que hacer
				 */
				printf("Comando recibido: JOURNAL\n\n");
			}
			break;
			default:
				puts("Recibi una operacion invalida...");
		}
	}
	close(cliente); // No me olvido de cerrar el socket que ya no voy a usar mas
	close(servidor); // No me olvido de cerrar el socket que ya no voy a usar mas

	return EXIT_SUCCESS;
}
