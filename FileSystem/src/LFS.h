#ifndef LFS_H_
#define LFS_H_
#include"Shared.h"

typedef struct{
	char* nombreTabla;
	pthread_attr_t *attrHilo;
}t_hiloCompactacion;

/*############ FUNCIONES ##############*/

//Discrimina el mensaje por parametro con un switch, y dependiendo el caso, retorna el valor esperado para ese caso
char* apiLissandra(char*);

//Indica si una tabla (una carpeta) existe en "puntoMontaje/Table/"
bool existeTabla(char* nombreTabla);

//Agrega los registros con una determinada key, con el mayor timestamp de bloques, .tmp y .tmpc a listaFiltro.
void agregarRegDeBloquesYTemps(t_list *lista, char* nombreTabla, u_int16_t key);

void crearDirectiorioDeTabla(char* nombreTabla);

//Ordena de mayor a menor los registros de una lista
bool ordenarDeMayorAMenorTimestampFinal(datos_a_compactar* r1, datos_a_compactar* r2);

//Convierte un t_registro* a un datos_a_compactar*
datos_a_compactar* convertirAStructDAC(t_registro *aux_memtable);

//Crea la carpeta y el archivo Metadata del FileSystem
void crearMetadataDeTabla(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
void crearBinDeTabla(char* nombreTabla, int cantParticiones);
void dumpDeTablas(t_list *memTableAux);
void agregarAMemTable(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp);
bool ordenarDeMayorAMenorTimestamp(t_registro* r1, t_registro* r2);
bool registro_IgualNombreTabla(t_hiloCompactacion *registro);
t_registro* convertirARegistroPuntero(t_registro r);
struct_select_respuesta convertirARespuestaSelect(datos_a_compactar* registro);
struct_describe_respuesta convertirARespuestaDescribe(char* consistencia, char* particiones, char* compactationTime);

//Crea un t_registro* con los datos dados por parametro.
t_registro* creadorRegistroPuntero(u_int16_t key, char* nombreTabla, uint64_t timeStamp, char* value);
char* intToString(long a);

char* encontreTabla(char* nombreTabla, DIR* path_buscado);
void crearArchivoTemporal(char* nombreTabla, char* particionTemp, int* bloques, int size, int cantidadBloques);
void dumpear(t_list *datosParaDump, char* carpetaNombre);
t_registro convertirAStruct(t_registro *registro);
void escribirEnBloque(FILE* f, t_registro* r);
bool esArchivoTemporal(char* nombre);
void discriminadorDeCasos(char* path, uint16_t key, uint64_t timestamp, char* value, int particion);
void casoParticular(char* path, int particion, FILE* bin, char* line, int renglon);
t_log* iniciar_logger();
void* servidor(argumentos_servidor* args);
void memoria_handler(int *socket_cliente);
void* dump(int tiempo_dump);
int ultimoBloques(char* bloques);
void iniciarArchivoConCeros(FILE* f);
char* agregarNuevoBloqueBin();
void insertarLinea(int bloqueNumero, char* linea);
int entraEnUltimoBloque(int size, char* line);
int sizeArchivo(FILE* archivo);
struct_describe_respuesta* convertirAPuntero(struct_describe_respuesta describe);
char** obtenerBloques(char* bloques);
char* obtenerBloquesDetabla(FILE* f);

//Borra todos los archivos de una tabla, sus bloques, y luego la tabla en si.
void borrarTabla(char* path);

//Quita el enlace en el bitmap sobre los bloques asignados a los .bin de la tabla
void quitarEnlaceBloques(char* bin_string);

//Carga los datos del bloque que coincidan con la Key
void cargarBloqueAListaSELECT(char* bloque, t_list* lista, char* append, u_int16_t key);

//Carga los datos del bloque que coincidan con la Key (Caso del ultimo bloque)
void cargarUltimoBloqueSELECT(char* bloque, t_list* lista, int size_lectura, char* append, u_int16_t key);

//Divide a linea y la ingresa a un struct datos_a_compactar, para agregarlo a lista.
void cargarLineaSELECT(char** linea, t_list* lista, u_int16_t key);

//Genera el hilo de compactacion para las tablas que ya existiecen.
void crearCompactacionTablasExistentes();

//Se devuelve el tiempo de compactacion de un archivo Metadata
int obtenerTiempoCompactacion(FILE* metadata);

//Crea los Mutex de las tablas ya existentes
void crearMutexTablasExistentes();

//Crea los mutex necesarios
void crearMutex();

//Corrobora que no sea "." ni ".."
bool esCarpetaValida(char* nombre);

#endif /* LFS_H_ */
