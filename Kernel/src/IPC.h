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

struct_select_respuesta selectAMemoria(struct_select paquete);
enum estados_create createAMemoria(struct_create paquete);


#endif /* IPC_H_ */
