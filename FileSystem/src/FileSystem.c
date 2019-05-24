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
void crearFicheroPadre();
void crearMetaDataFS();
void crearBitMapFS();
void crearTables();
void crearBloquesDatos();
bool existeTabla(char* nombreTabla);
char* u_intAString(u_int a);
char* intToString(int a);

void *fileSystem(t_config *config) {
	puts("Soy FileSystem");

	crearFicheroPadre();
	crearMetaDataFS();
	crearBitMapFS();
	crearTables();
	crearBloquesDatos();
	return 0;
}

void crearFicheroPadre(){
	if(mkdir(puntoMontaje, 0777) != 0){}
}
void crearMetaDataFS(){
	int tamanioTotal = strlen(puntoMontaje)+strlen("Metadata/")+strlen("Metadata.bin");
	char* path = malloc(tamanioTotal);
	strcpy(path, puntoMontaje);
	strcat(path, "Metadata/");
	if(mkdir(path, 0777) != 0){}
	strcat(path,"Metadata.bin");
	FILE *f = fopen(path, "w+");
	fclose (f);
	free(path);
}
void crearBitMapFS(){
	int tamanioTotal = strlen(puntoMontaje)+strlen("Metadata/Bitmap.bin");
	char* path = malloc(tamanioTotal);
	strcpy(path, puntoMontaje);
	strcat(path, "Metadata/Bitmap.bin");
	FILE *f = fopen(path, "w+");
	//<commons/bitarray.h>
	fclose(f);
	free(path);
}
void crearTables(){
	int tamanioTotal = strlen(puntoMontaje)+strlen("Table/");
	char* path = malloc(tamanioTotal);
	strcpy(path, puntoMontaje);
	strcat(path, "Table/");
	if(mkdir(path, 0777) != 0){}
	free(path);
}
void crearBloquesDatos(){
	int tamanioTotal = strlen(puntoMontaje)+strlen("Bloques/");
	char* path = malloc(tamanioTotal);
	strcpy(path, puntoMontaje);
	strcat(path, "Bloques/");
	if(mkdir(path, 0777) != 0){}
	free(path);
}

bool existeTabla(char* nombreTabla){
	int tamanioTotal = strlen(puntoMontaje)+strlen("Table/");
	char* path = malloc(tamanioTotal);
	strcpy(path, puntoMontaje);
	strcat(path,"Table/");

	int flag = 0;
	DIR* path_buscado = opendir(path);
	free(path);
	struct dirent* carpeta = readdir(path_buscado);;

	do{
		if(strcmp(nombreTabla, carpeta->d_name) == 0){
			flag = 1;
			break;
		}
		carpeta = readdir(path_buscado);
	}while(carpeta && flag == 0);

	if(flag == 1){
		return 1;
	}else{
		return 0;
	}
}

FILE* obtenerMetaDataLectura(char* nombreTabla){
	FILE* metadata;
	int tamanioTotal = strlen(puntoMontaje)+strlen("Table/")+strlen(nombreTabla)+strlen("Metadata/");
	char* path = malloc(tamanioTotal);
	strcpy(path, puntoMontaje);
	strcat(path,"Table/");
	strcat(path, nombreTabla);
	strcat(path,"Metadata");
	metadata = fopen(path, "r");
	free(path);
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

	char path_aux[100];
	char *path = config_get_string_value(config,"PUNTOMONTAJE");
	strcpy(path_aux,path);
	strcat(path_aux,"Table/");
	strcat(path_aux, nombreTabla);
	strcat(path_aux, "/");
	strcat(path_aux, nombre_bin);
	printf("%s\n", path_aux);
	bin = fopen(path_aux, "r+");
	free(nombre_bin);
	return bin;
}

void crearDirectiorioDeTabla(char* nombreTabla){
	int tamanioTotal = strlen(puntoMontaje)+strlen("Table/")+strlen(nombreTabla);
	char* path = malloc(tamanioTotal);
	strcpy(path, puntoMontaje);
	strcat(path,"Table/");
	strcat(path, nombreTabla);
	if(mkdir(path, 0777) != 0){}
	free(path);
}

void crearMetadataDeTabla(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	int tamanioTotal = strlen(puntoMontaje)+strlen("Table/")+strlen(nombreTabla)+strlen("/Metadata");
	char* path = malloc(tamanioTotal);
	strcpy(path, puntoMontaje);
	strcat(path,"Table/");
	strcat(path, nombreTabla);
	strcat(path, "/");
	strcat(path,"Metadata");
	FILE* metadata = fopen(path, "w");

	fputs("CONSISTENCY=", metadata);
	fputs(tipoConsistencia, metadata);fputs("\n", metadata);

	fputs("PARTITIONS=", metadata);
	char* cantPart = u_intAString(cantidadParticiones); //ROMPE
	fputs(cantPart, metadata);
	fputs("\n", metadata);
	free(cantPart);

	fputs("COMPACTION_TIME=", metadata);
	char* compactTime = u_intAString(compactionTime); //ROMPE
	fputs(compactTime, metadata);
	free(compactTime);
	free(path);
	fclose(metadata);
}

void crearBinDeTabla(char* nombreTabla, int cantParticiones){
	FILE* f;
	int tamanioTotal = strlen(puntoMontaje)+strlen("Table/")+strlen(nombreTabla)+strlen("10.bin");
	char* path = malloc(tamanioTotal);
	strcpy(path, puntoMontaje);
	strcat(path,"Table/");
	strcat(path, nombreTabla);
	strcat(path, "/");
	printf("%s \n", path);
	char *particion;
	for(int i=0;i<cantParticiones;i++){
		particion = intToString(i);
		strcat(particion,".bin");
		char* path_aux = malloc(tamanioTotal+ strlen(particion));
		strcpy(path_aux, path);
		strcat(path_aux, particion);
		f = fopen(path_aux, "w");
		fclose(f);
		free(path_aux);
	}
	free(path);
	free(particion);

}

// ##### FUNCIONES SECUNDARIAS #####

char* u_intAString(u_int a){
	u_int someInt = a; char str[12];
	char* string = malloc(sizeof(12));
	sprintf(str, "%u", someInt);
	strcpy(string,str);
	return string;
}

char* intToString(int a){
	int num = a; char str[12];
	char* string = malloc(strlen(str));
	sprintf(str, "%d", num);
	strcpy(string,str);
	return string;
}
