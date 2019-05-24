#ifndef LFS_H_
#define LFS_H_

#include<commons/log.h>
#include<commons/config.h>
#include<inttypes.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<readline/readline.h>
#include<pthread.h>
#include<unistd.h>
#include<string.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<signal.h>
#include<sys/socket.h>
#include<netdb.h>
#include<sys/stat.h>
#include<commons/bitarray.h>
#include <dirent.h>

#define SELECT "SELECT" 	//1
#define INSERT "INSERT"	 	//2
#define CREATE "CREATE" 	//3
#define DESCRIBE "DESCRIBE"	//4
#define DROP "DROP"			//5
#define EXIT "EXIT"
#define IP "127.0.0.1"

t_log* logger;
t_config* config;
char* puntoMontaje;
// ##### MemTable #####
typedef struct{
	double timeStamp;
	uint16_t key;
	char* value;
	char* nombre_tabla;
}t_registro;

typedef struct{
	double timeStamp;
	uint16_t key;
	char* value;
}t_registroBusqueda;

t_list *memTable;
int cont; 			// Contador de cantidad de registros en Memtable
int cantDumps;		//Contador de cantidad de dumps para hacer los archivos temporales
#endif /* LFS_H_ */
