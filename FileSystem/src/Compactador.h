#ifndef COMPACTADOR_H_
#define COMPACTADOR_H_
#include <stdbool.h>
#include"Shared.h"

//Renombra los .tmp a .tmpc y devuelve True si renombró a algun archivo
bool renombrarArchivosTemporales(char* path);

//Discrimina a los Tmpc para posteriormente depositar los datos
void analizarTmpc(char* path, char* nombreTabla);

//Corrobora si un string tiene el formato "A(...).tmpc"
bool esArchivoTemporalC(char* nombre);

//Agrega los datos de un archivo (.bin,.tmpc) a una lista para despues compactarlos.
void agregarDatosACompactar(t_list* lista, char* path);

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

//Filtra los registros que tengan mayor timestamp en la lista.
void filtrarRegistrosConMayorTimestamp(t_list* lista);

//Crea un archivo .bin en Table/NombreTabla
void crearArchivoBin(char* nombreTabla, int bin, int size, int* bloques, int cantidadBloques);

//Convierte los datos de la lista en un solo string con el formato "Timestamp;Key;Valor\nTimestamp;Key;Valor\nTimestamp;Key;Valor\n..."
char* lineasEnteraCompactacion(t_list* lista);

//Hace la carga de los datos del bloque a una lista, pero es el caso del ultimo bloque
void cargarUltimoBloque(char* bloque, t_list* lista, int size_lectura, char* append);

//Carga los datos del bloque a una lista, para posteriormente compactarla.
void cargarBloqueALista(char* bloque, t_list* lista, char* append);

#endif /* COMPACTADOR_H_ */
