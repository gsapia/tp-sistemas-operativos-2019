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


#define SELECT "SELECT"
#define INSERT "INSERT"
#define CREATE "CREATE"
#define DESCRIBE "DESCRIBE"
#define DROP "DROP"
#define JOURNAL "JOURNAL"
#define ADD "ADD"
#define RUN "RUN"
#define METRICS "METRICS"

t_log* logger;

#endif /* KERNEL_H_ */
