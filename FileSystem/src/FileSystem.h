#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
#include"Shared.h"

// Crea la carpeta principal "LISSANDRA_FS", que nos la dan en el .config
void crearFicheroPadre();

// Crea el archivo Metadata del FileSystem
void crearMetaDataFS();

// Crea el archivo "Bitmap.bin" del archivo de FileSystem
void crearBitMapFS();

//Crea el bitmap en si
void crearBitmap(char* path);

// Crea la carpeta "Table/" dentro de la carpeta del punto de montaje
void crearTables();

// Crea la carpeta "Bloques/" dentro de la carpeta del punto de montaje
void crearBloquesDatos();

//Crea los N cantidad de bloques, dados por el .config
void crearBloques(char* path);

//Toma todos los registros de la lista, y concatena todos los [Timestamp;Key;Valor] en un solo string
char* lineasEntera(t_list* lista);



#endif /* FILESYSTEM_H_ */
