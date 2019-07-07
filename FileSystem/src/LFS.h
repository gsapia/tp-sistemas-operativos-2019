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

typedef struct{
	char* nombreTabla;
	pthread_attr_t *attrHilo;
}t_hiloCompactacion;

t_list *memTable;
t_list *hilosCompactacion;
int cont; 			// Contador de cantidad de registros en Memtable
int cantDumps;		//Contador de cantidad de dumps para hacer los archivos temporales
int tamValue;
char* bitmap;
int blockSize;
int blocks;
t_bitarray* bitarray;

/*############ FUNCIONES ##############*/

//Lee una linea ingresada por teclado, y la envia por parametro a apiLissandra()
void* consola();

//Discrimina el mensaje por parametro con un switch, y dependiendo el caso, retorna el valor esperado para ese caso
char* apiLissandra(char*);

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
char* drop(char* nombreTabla);

//Indica si una tabla (una carpeta) existe en "puntoMontaje/Table/"
bool existeTabla(char* nombreTabla);

//Devuelve el archivo Metadata abierto para lectura, de una tabla
FILE* obtenerMetaDataLectura(char* nombreTabla);

//Obtiene la particion a la que pertenece la Key asociada
int obtenerParticion(char *nombreTabla, u_int16_t key);

//Devuelve el resto entre la divicion de key / particiones
int funcionModulo(int key, int particiones);

//Obtiene la cantidad de particiones del archivo Metadata
int obtenerParticiones(FILE* metadata);

//Agrega los registros con una determinada key, con el mayor timestamp de bloques, .tmp y .tmpc a listaFiltro.
void agregarRegDeBloquesYTemps(t_list *lista, char* nombreTabla, u_int16_t key);

//Agrega el registro con una determinada key el mayor timestamp de bloques a una lista.
void agregarRegistroBloques(char* nombreTabla, t_list *lista, u_int16_t key);

//Devuelve un el archivo ".bin" que corresponde a la particion enviada por parametro
FILE* obtenerBIN(int particion, char* nombreTabla);

//Agrega el registro con la key y el timestamp mas alto a la lista
void agregarRegistrosTempYTempc(char* nombreTabla, t_list* lista, u_int16_t key);

bool existeKeySELECT(u_int16_t key, char** bloques, t_list* lista);

char* obtenerPrimeraLinea(char* bloque);

char* lineasEntera(t_list* lista);
char** dividirLinea(char* linea);
void crearDirectiorioDeTabla(char* nombreTabla);
void crearMetadataDeTabla(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
void crearBinDeTabla(char* nombreTabla, int cantParticiones);
void dumpDeTablas(t_list *memTableAux);
void agregarAMemTable(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp);
uint64_t getTimestamp();
bool ordenarDeMayorAMenorTimestamp(t_registro* r1, t_registro* r2);
bool registro_IgualNombreTabla(t_hiloCompactacion *registro);
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
void crearBloques(char* path);
char* encontreTabla(char* nombreTabla, DIR* path_buscado);
void crearArchivoTemporal(char* nombreTabla, char* particionTemp, char** bloques, int size, int cantidadBloques);
void dumpear(t_list *datosParaDump, char* carpetaNombre);
t_registro convertirAStruct(t_registro *registro);
void escribirEnBloque(FILE* f, t_registro* r);
bool renombrarArchivosTemporales(char* path);
bool esArchivoTemporal(char* nombre);
void analizarTmpc(char* path, char* nombreTabla);
void discriminadorDeCasos(char* path, uint16_t key, uint64_t timestamp, char* value, int particion);
void casoParticular(char* path, int particion, FILE* bin, char* line, int renglon);
t_config *leer_config();
t_log* iniciar_logger();
void* servidor(argumentos_servidor* args);
void memoria_handler(int *socket_cliente);
void* fileSystem();
void* dump(int tiempo_dump);
bool esArchivoTemporalC(char* nombre);
int obtenerUltimoBloqueBin(FILE* bin);
int ultimoBloques(char* bloques);
void crearBitmap(char* path);
void iniciarArchivoConCeros(FILE* f);
char* agregarNuevoBloqueBin();
void insertarLinea(int bloqueNumero, char* linea);
int entraEnUltimoBloque(int size, char* line);
int sizeArchivo(FILE* archivo);
char* existeKeyEnBloques(uint16_t key_tmpc, FILE* binTabla);
int obtenerSizeBin(FILE* bin);
uint64_t stringToLong(char* strToInt);
struct_describe_respuesta* convertirAPuntero(struct_describe_respuesta describe);
char** obtenerBloques(char* bloques);
char* obtenerBloquesDetabla(FILE* f);
//Indica true, si la key pertenece al bloque asignado con bloque_numero. False en caso contrario.
char* existeKey(u_int16_t key, char** bloques);
bool entraEnBloque(char* line, int bloque);
void modificarBinTabla(char* linea, char* nuevoBloque, FILE* bin, char* path_bin);
char* getNewBloque();
//Calcula el tamañano del bloque: "puntoMontaje/Bloques/numeroBloque.bin"
int calcularTamanoBloque(int numeroBloque);

//Obtiene la ultima linea de un bloque y la retorna
char* obtenerUltimaLinea(char* bloque);

//Borra todos los archivos de una tabla, sus bloques, y luego la tabla en si.
void borrarTabla(char* path);

//Quita el enlace en el bitmap sobre los bloques asignados a los .bin de la tabla
void quitarEnlaceBloques(char* bin_string);

//Libera la memoria de cada posicion del array, y el puntero al array
void liberarArrayString(char** array);
#endif /* LFS_H_ */
