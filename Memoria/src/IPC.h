#ifndef IPC_H_
#define IPC_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "Memoria.h"
#include "serializacion.h"


void initCliente();
void closeCliente();
void servidor();

t_list* intercambiar_tabla_gossiping(t_memoria memoria);

struct_select_respuesta selectAFS(struct_select paquete);
enum estados_insert insertAFS(struct_insert paquete);
enum estados_create createAFS(struct_create paquete);
struct_describe_respuesta describeAFS(struct_describe paquete);
struct_describe_global_respuesta describeGlobalAFS();
enum estados_drop dropAFS(struct_drop paquete);

#endif /* IPC_H_ */
