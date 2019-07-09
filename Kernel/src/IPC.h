//InterProcess Comunication Kernel Header

#ifndef IPC_H_
#define IPC_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "serializacion.h"

void initCliente();
void closeCliente();

int conectar(char* ip, uint16_t puerto);

void addMetadata(char* nombre_tabla, struct_describe_respuesta* describe);

void refreshMetadata();

struct_select_respuesta selectAMemoria(struct_select paquete, t_memoria* memoria);
enum estados_insert insertAMemoria(struct_insert paquete, t_memoria* memoria);
enum estados_create createAMemoria(struct_create paquete);
struct_describe_respuesta describeAMemoria(struct_describe paquete, t_memoria * memoria);
struct_describe_global_respuesta describeGlobalAMemoria(t_memoria* memoria);
enum estados_journal journalMemoria(t_memoria* memoria);

#endif /* IPC_H_ */
