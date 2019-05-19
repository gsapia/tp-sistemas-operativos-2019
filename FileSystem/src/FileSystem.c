/*
-Estrutura de Arbol de directorios. Su inicio nos lo da el Archivo de configuracion.

##### METADATA #####
Contiene:
	-Block_size: tamaño en bytes de cada bloque
	-Blocks: cant de bloques del FileSystem
	-Magic_Number: string fijo con el valor "LFS".
Se encontrara en [Punto_Montaje]/Metadata/Metadata.bin

##### BITMAP #####
Archivo binario donde solamente exisitra un bitmap, que representara el estado de los bloques dentro del FS.
1 = bloque ocupado
0 = bloque libre
Se encontrara en [Punto_Montaje]/Metadata/Bitmap.bin
*/

#include "LFS.h"
t_config *leer_config();
void crearMetaDataFS(char path[]);
void crearBitMapFS(char path[]);
void crearTables(char path[]);
void crearBloquesDatos(char path[]);
bool existeTabla(char* nombreTabla);
FILE* obtenerMetaData(char* nombreTabla);

void *fileSystem(t_config *config) {
	puts("Soy FileSystem");

	//Creacion del Fichero Padre
	char path[100];
	char *path_aux = config_get_string_value(config,"PUNTOMONTAJE");
	strcpy(path,path_aux);
	char nombreArchivo[100];
	if(mkdir(path, 0777) != 0){
//		printf("No se pudo crear Carpeta\n");
	}
	crearMetaDataFS(path);
	crearBitMapFS(path);
	crearTables(path);
	crearBloquesDatos(path);
	strcat(path, nombreArchivo);
	//FILE *f = fopen(path, "w+");
}

void crearMetaDataFS(char path[]){
	char path_aux[100];
	strcpy(path_aux,path);
	strcat(path_aux, "Metadata/");
	if(mkdir(path_aux, 0777) != 0){
//			printf("No se pudo crear la carpeta /Metadata (Puede que ya este creada)\n");
	}
	strcat(path_aux,"Metadata.bin");
	FILE *f = fopen(path_aux, "w+");
}
void crearBitMapFS(char path[]){
	char path_aux[100];
	strcpy(path_aux,path);
	strcat(path_aux, "Metadata/");
	if(mkdir(path_aux, 0777) != 0){
//			printf("No se pudo crear la carpeta /Metadata (Puede que ya este creada)\n");
	}
	strcat(path_aux,"Bitmap.bin");
	FILE *f = fopen(path_aux, "w+");
	//<commons/bitarray.h>
}
void crearTables(char path[]){
	char path_aux[100];
	strcpy(path_aux,path);
	strcat(path_aux, "Table/");
	if(mkdir(path_aux, 0777) != 0){
//		printf("No se pudo crear la carpeta /Tables.\n");
	}
}
void crearBloquesDatos(char path[]){
	char path_aux[100];
	strcpy(path_aux,path);
	strcat(path_aux, "Bloques/");
	if(mkdir(path_aux, 0777) != 0){
//		printf("No se pudo crear /Bloques\n");
	}
}

bool existeTabla(char* nombreTabla){
	char pathaux[100];
	char *path_aux = config_get_string_value(config,"PUNTOMONTAJE");
	strcpy(pathaux,path_aux);
	strcat(pathaux,"Table/");
	strcat(pathaux,nombreTabla);
	printf("%s EXISTE \n",pathaux);
	//Chequear si esa tabla ya existe
	return true;
}

FILE* obtenerMetaDataLectura(char* nombreTabla){
	FILE* metadata;
	char pathaux[100];
	char *path_aux = config_get_string_value(config,"PUNTOMONTAJE");
	strcpy(pathaux,path_aux);
	strcat(pathaux,"Table/");
	strcat(pathaux, nombreTabla);
	strcat(pathaux, "/Metadata");
	printf("%s\n", pathaux);
	metadata = fopen(pathaux, "r");
	return metadata;
}

int obtenerParticiones(FILE* metadata){
	char aux[100];int particiones;
	fgets(aux, 100, metadata);
	fgets(aux, 100, metadata);
	char** part = string_split(aux, "=");
	particiones = atoi(part[1]);
	return particiones;
}

FILE* obtenerBIN(int particion, char* nombreTabla){
	FILE* bin;
	int length = snprintf( NULL, 0, "%d", particion);
	char* nombre_bin = malloc( length + 1 );
	snprintf(nombre_bin, length + 1, "%d", particion);
	strcat(nombre_bin, ".bin");

	char pathaux[100];
	char *path_aux = config_get_string_value(config,"PUNTOMONTAJE");
	strcpy(pathaux,path_aux);
	strcat(pathaux,"Table/");
	strcat(pathaux, nombreTabla);
	strcat(pathaux, "/");
	strcat(pathaux, nombre_bin);
	printf("%s\n", pathaux);
	bin = fopen(pathaux, "r+");
	free(nombre_bin);
	return bin;
}
