/*
-Estrutura de Arbol de directorios. Su inicio nos lo da el Archivo de configuracion.

##### METADATA #####
Contiene:
	-Block_size: tamaño en bytes de cada bloque
	-Blocks: cant de bloques del FileSystem
	-Magic_Number: string fijo con el valor "LFS".
Se encontrara en [Punto_Montaje]/Metadata/Metadata.bin

##### BITMAP #####
Archivo binario donde solamente existira un bitmap, que representara el estado de los bloques dentro del FS.
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
char* encontreTabla(char* nombreTabla, DIR* path_buscado);
FILE* crearArchivoTemporal(char* nombreTabla, char* particionTemp);
t_list* tieneDatosParaDump(t_list *memTableAux, char* carpetaNombre);
void dumpear(t_list *datosParaDump, char* carpetaNombre);
t_registro convertirAStruct(t_registro *registro);

// Hilo del FILESYSTEM (Crea las carpetas necesarias)
void *fileSystem() {
	log_info(logger, "> FileSystem iniciado.");

	crearFicheroPadre();
	crearMetaDataFS();
	crearBitMapFS();
	crearTables();
	crearBloquesDatos();
	log_info(logger, "> FileSystem finalizado.");
	return 0;
}

// Crea la carpeta principal "LISSANDRA_FS", que nos la dan en el .config
void crearFicheroPadre(){
	if(mkdir(puntoMontaje, 0777) != 0){}
}

// Crea el archivo Metadata del FileSystem
void crearMetaDataFS(){
	char* path = string_from_format("%sMetadata/", puntoMontaje);
	if(mkdir(path, 0777) != 0){}
	path = string_from_format("%sMetadata.bin", path);
	FILE *f = fopen(path, "w+");
	fclose (f);
	free(path);
}

// Crea el archivo "Bitmap.bin" del archivo de FileSystem
void crearBitMapFS(){
	char* path = string_from_format("%sMetadata/Bitmap.bin", puntoMontaje);
	FILE* f = fopen(path, "w+");
	fclose(f);
	free(path);
}
// Crea la carpeta "Table/" dentro de la carpeta del punto de montaje
void crearTables(){
	char* path = string_from_format("%sTable/", puntoMontaje);
	if(mkdir(path, 0777) != 0){}
	free(path);
}

// Crea la carpeta "Bloques/" dentro de la carpeta del punto de montaje
void crearBloquesDatos(){
	char* path = string_from_format("%sBloques/", puntoMontaje);
	if(mkdir(path, 0777) != 0){}
	free(path);
}

// Corrobora la existencia de una carpeta dentro de la carpeta "Table/"
bool existeTabla(char* nombreTabla){
	char* path = string_from_format("%sTable/", puntoMontaje);
	DIR* path_buscado = opendir(path);
	free(path);

	if(encontreTabla(nombreTabla, path_buscado)){
		return true;
	}else{
		return false;
	}
}

// Se fija si dentro del path_buscado, se encuentra la tabla (nombreTabla). Si se encuentra
// se devuelve el nombre encontrado.
char* encontreTabla(char* nombreTabla, DIR* path_buscado){
	struct dirent* carpeta = readdir(path_buscado);
	int flag = 0;
	do{
		if(strcmp(nombreTabla, carpeta->d_name) == 0){
			flag = 1;
			break;
		}
		carpeta = readdir(path_buscado);
	}while(carpeta && flag == 0);

	if(flag == 1){
		return carpeta->d_name;
	}else{return NULL;}
}

// Va fijandose tabla por tabla, dentro de "Table/", si es necesario el dumpeo de datos
void dumpDeTablas(t_list *memTableAux){
	t_list* datosParaDump = list_create();
	int flag = 0; // Sirve para indicar que se hizo un dump y sumarle a la variable cantDumps (Sirve mas para testeos).
	char *path = string_from_format("%sTable/", puntoMontaje);
	DIR* path_buscado = opendir(path);
	free(path);
	struct dirent* carpeta = readdir(path_buscado);


	while(carpeta){
		if(strcmp(carpeta->d_name, ".") && strcmp(carpeta->d_name, "..")){ //Si el nombre de la carpeta es distinto a "." y ".."
			datosParaDump = tieneDatosParaDump(memTableAux, carpeta->d_name);
			if(!list_is_empty(datosParaDump)){
				log_info(logger, "%s tiene datos para Dumpear", carpeta->d_name);
				dumpear(datosParaDump, carpeta->d_name);
				flag = 1;
			}else{
				log_info(logger, "%s no tiene datos para Dumpear", carpeta->d_name);
			}
		}
		carpeta = readdir(path_buscado);
	}
	if(flag==1){
		cantDumps++;
	}
}

// Se fija si una carpeta tiene datos para dumpear en la MemTable. Si los tiene, los devuelve
// Sino, devuelve NULL
t_list* tieneDatosParaDump(t_list *memTableAux, char* carpetaNombre){
	t_list* datosDump = list_create();
	t_registro *registro;

	while(!list_is_empty(memTableAux)){
		registro = list_remove(memTableAux, 0);
		if(!strcmp(registro->nombre_tabla, carpetaNombre)){
			log_info(logger, "Añado el value: %s", registro->value);
			list_add(datosDump, registro);
		}else{
			list_add(memTableAux, registro);
		}
	}
	return datosDump;
}

//Dumpeo los datos dentro de un archivo ".tmp" que se encuentra dentro de "/Table/carpetaNombre"
void dumpear(t_list *datosParaDump, char* carpetaNombre){
	char* particionTemp = malloc(12);
	particionTemp = intToString(cantDumps);
	FILE* temp = crearArchivoTemporal(carpetaNombre, particionTemp);
	t_registro *registro; t_registro aux;

	while(!list_is_empty(datosParaDump)){
		registro = list_remove(datosParaDump,0);
		aux = convertirAStruct(registro);
	    fwrite(&aux, sizeof(t_registro), 1, temp);
	}
/*	fseek(temp, 0, SEEK_SET);
	while(fread(&aux2, sizeof(t_registro), 1, temp)){
		log_info(logger, "%d", aux2.key);
		log_info(logger, "%s", aux2.value);
		log_info(logger, "%lf", aux2.timeStamp);

	}
*/
	fclose(temp);
	free(particionTemp);
}

FILE* crearArchivoTemporal(char* nombreTabla, char* particionTemp){
	char* path = string_from_format("%sTable/%s/A%s.tmp", puntoMontaje, nombreTabla, particionTemp);
	FILE* f = fopen(path, "wb+");
	free(path);
	return f;
}

FILE* obtenerMetaDataLectura(char* nombreTabla){
	char* path = string_from_format("%sTable/%s/Metadata", puntoMontaje, nombreTabla);
	FILE* metadata = fopen(path, "r");
	free(path);
	return metadata;
}

// Se obtiene la cantidad de particiones (.bin) que existen en una carpeta, desde
// el archivo Metadata
int obtenerParticiones(FILE* metadata){
	char aux[20];int particiones;
	fgets(aux, 20, metadata);
	fgets(aux, 20, metadata);
	char** part = string_split(aux, "=");
	particiones = atoi(part[1]);
	return particiones;
}

//Devuelve un el archivo ".bin" que corresponde a la particion enviada por parametro
FILE* obtenerBIN(int particion, char* nombreTabla){
	char* nombre_bin = intToString(particion);
	char* path = string_from_format("%sTable/%s/%s.bin", puntoMontaje, nombreTabla, nombre_bin);
	FILE *bin = fopen(path, "r+");
	free(path);
	free(nombre_bin);
	return bin;
}

// Crea la carpeta dentro de "Table/" con el nombreTabla
void crearDirectiorioDeTabla(char* nombreTabla){
	char* path = string_from_format("%sTable/%s", puntoMontaje, nombreTabla);
	if(mkdir(path, 0777) != 0){}
	free(path);
}

// Crea el archivo Metadata y no incializa con los parametros enviados.
void crearMetadataDeTabla(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	char* path = string_from_format("%sTable/%s/Metadata", puntoMontaje, nombreTabla);
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

// Crea todos los archivos ".bin" necesarios en la tabla correspondiente.
void crearBinDeTabla(char* nombreTabla, int cantParticiones){
	FILE* f;
	char* path = string_from_format("%sTable/%s/", puntoMontaje, nombreTabla);
	char* particion;
	for(int i=0;i<cantParticiones;i++){
		particion = intToString(i);
		char* path_aux = string_from_format("%s%s.bin", path, particion);
		f = fopen(path_aux, "wb");
		fclose(f);
		free(path_aux);
	}
	free(path);
	free(particion);

}

// ##### FUNCIONES SECUNDARIAS #####

// Convierte un u_int en un string
char* u_intAString(u_int a){
	u_int someInt = a; char str[12];
	char* string = malloc(sizeof(12));
	sprintf(str, "%u", someInt);
	strcpy(string,str);
	return string;
}

// Convierte un entero en un string
char* intToString(int a){
	int num = a; char str[12];
	char* string = malloc(strlen(str));
	sprintf(str, "%d", num);
	strcpy(string,str);
	return string;
}

// Convierte un t_registro* a un t_registro
t_registro convertirAStruct(t_registro *registro){
	t_registro r;
	r.key = registro->key;
	strcpy(r.value, registro->value);
	r.timeStamp = registro->timeStamp;
	return r;
}
