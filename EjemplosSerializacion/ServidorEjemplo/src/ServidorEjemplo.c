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


				// Para el ejemplo, respondo con un registro inventado cualquiera solo si la key es 5:
				puts("Respondiendo SELECT");
				struct_select_respuesta registro;
				if(paquete.key == 5){
					registro.estado = ESTADO_SELECT_OK;
					registro.valor = "Test";
					registro.timestamp = 123456789;
				}
				else{
					registro.estado = ESTADO_SELECT_ERROR_KEY;
				}

				// Y lo mando:
				enviar_registro(cliente, registro);

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

				// Para el ejemplo, respondo con un estado OK solo si la tabla es TABLA1:
				puts("Respondiendo INSERT");
				uint16_t estado;
				if(!strcmp(paquete.nombreTabla, "TABLA1")){
					estado = ESTADO_CREATE_OK;
				}
				else{
					estado = ESTADO_CREATE_ERROR_TABLAEXISTENTE;
				}

				// Y lo mando:
				responder_insert(cliente, estado);

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

				// Para el ejemplo, respondo con un estado OK solo si la tabla es TABLA1:
				puts("Respondiendo CREATE");
				uint16_t estado;
				if(!strcmp(paquete.nombreTabla, "TABLA1")){
					estado = ESTADO_CREATE_OK;
				}
				else{
					estado = ESTADO_CREATE_ERROR_TABLAEXISTENTE;
				}

				// Y lo mando:
				responder_create(cliente, estado);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				free(paquete.nombreTabla);
			}
			break;
			case DESCRIBE_GLOBAL:
			{
				puts("Recibi un DESCRIBE GLOBAL");

				/*
				 * Depues haria lo que tenga que hacer
				 */
				printf("Comando recibido: DESCRIBE\n\n");

				// Para el ejemplo respondemos con la metadata de 2 tablas
				struct_describe_global_respuesta respuesta;
				respuesta.estado = ESTADO_DESCRIBE_OK;
				respuesta.describes = dictionary_create();

				struct_describe_respuesta* tabla1 = malloc(sizeof(struct_describe_respuesta));
				tabla1->estado = ESTADO_DESCRIBE_OK;
				tabla1->consistencia = SC;
				tabla1->particiones = 5;
				tabla1->tiempo_compactacion = 60000;
				dictionary_put(respuesta.describes, "TABLA1", tabla1);

				struct_describe_respuesta* tabla2 = malloc(sizeof(struct_describe_respuesta));
				tabla2->estado = ESTADO_DESCRIBE_OK;
				tabla2->consistencia = SHC;
				tabla2->particiones = 7;
				tabla2->tiempo_compactacion = 60000;
				dictionary_put(respuesta.describes, "TABLA2", tabla2);

				// Y lo mando
				enviar_respuesta_describe_global(cliente, respuesta);

				// Por ultimo, y sabiendo que no voy a usar mas el paquete, libero la memoria dinamica (MUCHO MUY IMPORTANTE)
				dictionary_destroy_and_destroy_elements(respuesta.describes, free);
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

				// Para el ejemplo respondemos cualquier cosa
				struct_describe_respuesta respuesta;
				respuesta.estado = ESTADO_DESCRIBE_OK;
				respuesta.consistencia = SC;
				respuesta.particiones = 5;
				respuesta.tiempo_compactacion = 60000;

				// Y lo mando
				enviar_respuesta_describe(cliente, respuesta);

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

				responder_drop(cliente, ESTADO_DROP_OK);

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

				responder_journal(cliente, ESTADO_JOURNAL_OK);
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
