#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<arpa/inet.h>           //Implemento librerias para utilizar Sockets de flujo.
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include <pthread.h>
#include<readline/history.h>



//Borre los define de kernel.h dado que estan definidos en serializacion.h


struct ConfigKernel{

	char* ip_memoria;
	int puerto_memoria;
	int quantum;
	int multiprocesamiento;
	int refresh_metadata;
	int retardo_ciclico;

}config;


t_log* logger;

#endif /* KERNEL_H_ */
