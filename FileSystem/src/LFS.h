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
#include<dirent.h>
#include<arpa/inet.h>
#include <sys/time.h>
#include "serializacion.h"
t_log* logger;
t_config* config;
char* puntoMontaje;
// ##### MemTable #####

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
	uint64_t timeStamp;
	uint16_t key;
	char* value;
}t_registroBusqueda;

t_list *memTable;
int cont; 			// Contador de cantidad de registros en Memtable
int cantDumps;		//Contador de cantidad de dumps para hacer los archivos temporales
int tamValue;


//############ FUNCIONES ##############
void* consola();
char* apiLissandra(char*);
void *compactacion(argumentos_compactacion *args);
struct_select_respuesta selects(char* nombreTabla, u_int16_t key);
char* insert(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp);
uint16_t create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
struct_describe_respuesta describe(char* nombreTabla);
char* drop(char* nombreTabla);
bool existeTabla(char* nombreTabla);
FILE* obtenerMetaDataLectura(char* nombreTabla);
int obtenerParticion(char *nombreTabla, u_int16_t key);
int funcionModulo(int key, int particiones);
int obtenerParticiones(FILE* metadata);
void agregarRegDeBinYTemps(t_list *listaFiltro, char* nombreTabla, u_int16_t key, int particion_busqueda);
FILE* obtenerBIN(int particion, char* nombreTabla);
void crearDirectiorioDeTabla(char* nombreTabla);
void crearMetadataDeTabla(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
void crearBinDeTabla(char* nombreTabla, int cantParticiones);
void dumpDeTablas(t_list *memTableAux);
void agregarAMemTable(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp);
uint64_t getTimestamp();
bool ordenarDeMayorAMenorTimestamp(t_registro* r1, t_registro* r2);
void obtenerRegistrosDeTable(t_list *listaFiltro, u_int16_t key, int particion_busqueda, char* nombreTabla);
t_registro* convertirARegistroPuntero(t_registro r);
struct_select_respuesta convertirARespuestaSelect(t_registro* mayor);
struct_describe_respuesta convertirARespuestaDescribe(char* consistencia, char* particiones, char* compactationTime);
void agregarRegistroMayorTimeStamDeArchivo(FILE* f, t_list *lista, u_int16_t key);
t_registro* creadorRegistroPuntero(u_int16_t key, char* nombreTabla, uint64_t timeStamp, char* value);
char* intToString(long a);
void crearFicheroPadre();
void crearMetaDataFS();
void crearBitMapFS();
void crearTables();
void crearBloquesDatos();
char* encontreTabla(char* nombreTabla, DIR* path_buscado);
FILE* crearArchivoTemporal(char* nombreTabla, char* particionTemp);
void dumpear(t_list *datosParaDump, char* carpetaNombre);
t_registro convertirAStruct(t_registro *registro);
void escribirEnArchivo(FILE* f, t_registro* r);
bool renombrarArchivosTemporales(char* path);
bool esArchivoTemporal(char* nombre);
void analizarTmpc(char* path, char* nombreTabla);
void discriminadorDeCasos(char* path, uint16_t key, uint64_t timestamp, char* value, int particion);
void casoParticular(char* path, int particion, FILE* bin, char* line, int renglon);
t_config *leer_config();
t_log* iniciar_logger();
void* servidor(argumentos_servidor* args);
void* fileSystem();
void* dump(int tiempo_dump);
bool esArchivoTemporalC(char* nombre);
int obtenerUltimoBloqueBin(char* path, int particion);
int* obtenerBloques(char* bloques);
int ultimoBloques(char* bloques);

#endif /* LFS_H_ */
