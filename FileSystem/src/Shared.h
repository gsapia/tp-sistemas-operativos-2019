#ifndef SHARED_H_
#define SHARED_H_

#include<commons/log.h>
#include<commons/config.h>
#include<inttypes.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<commons/txt.h>
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
#include<dirent.h>
#include<arpa/inet.h>
#include<sys/time.h>
#include"serializacion.h"
#include<sys/types.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<commons/collections/dictionary.h>

t_log* logger;
t_config* config;
char* puntoMontaje;
t_list *memTable;
int cantDumps;		//Contador de cantidad de dumps para hacer los archivos temporales
int tamValue;
char* bitmap;
int blockSize;
int blocks;
t_bitarray* bitarray;
t_dictionary* diccionario;

typedef struct{
	uint16_t puerto_escucha;
	int tamValue;
}argumentos_servidor;

typedef struct{
	int compactation_time;
	char* nombreTabla;
}argumentos_compactacion;

typedef struct{
	uint64_t timeStamp;
	uint16_t key;
	char* value;
	char* nombre_tabla;
}t_registro;

typedef struct{
	char* timestamp;
	char* key;
	char* value;
}datos_a_compactar;

// ######### FUNCIONES #########

//Corrobora que un archivo no sea ".", ni "..", ni metadata
bool esArchivoValido(char* nombre_archivo);

// Hilo del FILESYSTEM (Crea las carpetas necesarias)
void* fileSystem();

//Lee una linea ingresada por teclado, y la envia por parametro a apiLissandra()
void* consola();

//Realiza la compactacion de una carpeta especifica
void *compactacion(argumentos_compactacion *args);

//Realiza la operacion SELECT
struct_select_respuesta selects(char* nombreTabla, u_int16_t key);

//Realiza la operacion INSERT
enum estados_insert insert(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp);

//Realiza la operacion CREATE
uint16_t create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);

//Realiza la operacion DESCRIBE
struct_describe_respuesta describe(char* nombreTabla);

//Realiza la operacion DESCRIBE en todas las tablas existentes
struct_describe_global_respuesta describe_global();

//Realiza la operacion DROP
enum estados_drop drop(char* nombreTabla);

//Devuelve el archivo Metadata abierto para lectura, de una tabla
FILE* obtenerMetaDataLectura(char* nombreTabla);

//Devuelve el resto entre la divicion de key / particiones
int funcionModulo(int key, int particiones);

//Obtiene la cantidad de particiones del archivo Metadata
int obtenerParticiones(FILE* metadata);

//Devuelve un el archivo ".bin" que corresponde a la particion enviada por parametro
FILE* obtenerBIN(int particion, char* nombreTabla);

//Obtiene la primera linea de un bloque
char* obtenerPrimeraLinea(char* bloque);

//Devuelve el size que tenga en ese momento un archivo (.bin, .tmp, .tmpc).
int obtenerSizeBin(FILE* f);

//Devuelve un array de string con los bloques de un archivo (.bin, .tmp, .tmpc).
char** obtenerBloquesBin(FILE* f);

//Corrobora si el bloque, es el ultimo del array
bool esUltimoBloque(char** bloques, int indice);

//Convierte a un string en un Long Long.
uint64_t stringToLongLong(char* strToInt);

//Libera la memoria de cada posicion del array, y el puntero al array
void liberarArrayString(char** array);
#endif /* SHARED_H_ */
