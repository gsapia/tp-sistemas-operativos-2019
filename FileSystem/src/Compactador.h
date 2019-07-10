#ifndef COMPACTADOR_H_
#define COMPACTADOR_H_
#include <stdbool.h>

typedef struct{
	char* timestamp;
	char* key;
	char* value;
}datos_a_compactar;

//Calcula el tamañano del bloque: "puntoMontaje/Bloques/numeroBloque.bin"
int calcularTamanoBloque(int numeroBloque);

//Renombra los .tmp a .tmpc y devuelve True si renombró a algun archivo
bool renombrarArchivosTemporales(char* path);

//Discrimina a los Tmpc para posteriormente depositar los datos
void analizarTmpc(char* path, char* nombreTabla);

//Corrobora si un string tiene el formato "A(...).tmpc"
bool esArchivoTemporalC(char* nombre);

//Devuelve el ultimo bloque usado
int obtenerUltimoBloqueBin(FILE* bin);

//Agrega los datos de un archivo (.bin,.tmpc) a una lista para despues compactarlos.
void agregarDatosACompactar(t_list* lista, char* path);

//Devuelve el size que tenga en ese momento un archivo (.bin, .tmp, .tmpc).
int obtenerSizeBin(FILE* f);

//Devuelve un array de string con los bloques de un archivo (.bin, .tmp, .tmpc).
char** obtenerBloquesBin(FILE* f);

//Carga los datos del bloque a una lista, para posteriormente compactarla.
void cargarBloqueALista(char* bloque, t_list* lista, char* append);

//Corrobora que un archivo no sea "." ni ".."
bool esArchivoValido(char* nombre_archivo);

//Corrobora si el bloque, es el ultimo del array
bool esUltimoBloque(char** bloques, int indice);

//Hace la carga de los datos del bloque a una lista, pero es el caso del ultimo bloque
void cargarUltimoBloque(char* bloque, t_list* lista, int size_lectura, char* append);

//Divide a linea y la ingresa a un struct datos_a_compactar, para agregarlo a lista.
void cargarLinea(char** linea, t_list* lista);

//Libera los bloques de un archivo
void liberarBloques(char* path_bin);

//Libera un bloque del bitarray.
void freeBloque(char* bloque);

//Escribe los datos de la lista en sus correspondientes .bin
void escribirDatosLista(t_list* lista, char* nombreTabla);

//Corrobora si existe alguna linea mas en el archive
bool esUltimaLinea(FILE* f);

//Convierte a un string en un Long Long.
uint64_t stringToLongLong(char* strToInt);

//Filtra los registros que tengan mayor timestamp en la lista.
void filtrarRegistrosConMayorTimestamp(t_list* lista);

//Crea un archivo .bin en Table/NombreTabla
void crearArchivoBin(char* nombreTabla, int bin, int size, int* bloques, int cantidadBloques);

//Convierte los datos de la lista en un solo string con el formato "Timestamp;Key;Valor\nTimestamp;Key;Valor\nTimestamp;Key;Valor\n..."
char* lineasEnteraCompactacion(t_list* lista);

#endif /* COMPACTADOR_H_ */
