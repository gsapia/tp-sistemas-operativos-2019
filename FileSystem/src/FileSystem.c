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

// Hilo del FILESYSTEM (Crea las carpetas necesarias)
void *fileSystem() {
	crearFicheroPadre();
	crearMetaDataFS();
	crearBitMapFS();
	crearTables();
	crearBloquesDatos();
	log_trace(logger, "FileSystem ha creado las carpetas necesarias.");
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
	char* path2 = string_from_format("%sMetadata.bin", path);
	FILE *f = fopen(path2, "w");
	
	fputs("BLOCK_SIZE=", f);
	fputs("64", f);
	fputs("\n", f);
	
	fputs("BLOCKS=", f);
	fputs("5192", f);
	fputs("\n", f);
	
	fputs("MAGIC_NUMBER=", f);
	fputs("LISSANDRA", f);
	fputs("\n", f);
	
	fclose (f);
	free(path);
	free(path2);
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
		closedir(path_buscado);
		return true;
	}else{
		closedir(path_buscado);
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
	t_list* datosParaDump = NULL;
	int flag = 0; // Sirve para indicar que se hizo un dump y sumarle a la variable cantDumps (Sirve mas para testeos).
	char *path = string_from_format("%sTable/", puntoMontaje);
	DIR* path_buscado = opendir(path);
	free(path);
	struct dirent* carpeta = readdir(path_buscado);

	while(carpeta){
		if(strcmp(carpeta->d_name, ".") && strcmp(carpeta->d_name, "..")){ //Si el nombre de la carpeta es distinto a "." y ".."
			
			bool tieneDatosParaDump(t_registro* reg){return !strcmp(reg->nombre_tabla, carpeta->d_name);}
			datosParaDump = list_filter(memTableAux, (_Bool (*)(void*))tieneDatosParaDump); //Filtro de la memtable los registros tiene que ser dumpeados en cada Tabla/.

			if(!list_is_empty(datosParaDump)){
				dumpear(datosParaDump, carpeta->d_name);
				list_destroy(datosParaDump);
				datosParaDump = NULL;
				flag = 1;
			}else{
				list_destroy(datosParaDump);
			}
		}
		carpeta = readdir(path_buscado);
	}
	if(flag==1){
		cantDumps++;
	}
	closedir(path_buscado);
}

//Dumpeo los datos dentro de un archivo ".tmp" que se encuentra dentro de "/Table/carpetaNombre"
void dumpear(t_list *datosParaDump, char* carpetaNombre){
	char* particionTemp;
	particionTemp = intToString(cantDumps);
	FILE* temp = crearArchivoTemporal(carpetaNombre, particionTemp);
	free(particionTemp);
	t_registro *registro;

	while(!list_is_empty(datosParaDump)){
		registro = list_remove(datosParaDump,0);
		escribirEnArchivo(temp, registro);
	}
	free(registro->nombre_tabla);
	free(registro->value);
	free(registro);
	fclose(temp);
}

void escribirEnArchivo(FILE* f, t_registro* r){
	char* timestamp = intToString(r->timeStamp);
	char* key = intToString(r->key);
	char* linea = string_from_format("%s;%s;%s\n", timestamp, key, r->value); // [TIMESTAMP];[KEY];[VALUE]
	fputs(linea, f);
	free(timestamp);
	free(key);
	free(linea);
}

FILE* crearArchivoTemporal(char* nombreTabla, char* particionTemp){
	char* path = string_from_format("%sTable/%s/A%s.tmp", puntoMontaje, nombreTabla, particionTemp);
	FILE* f = fopen(path, "w+");
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
	free(part[1]);
	free(part[0]);
	free(part);
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
	fputs(tipoConsistencia, metadata);
	fputs("\n", metadata);

	fputs("PARTITIONS=", metadata);
	char* cantPart = intToString(cantidadParticiones); //ROMPE
	fputs(cantPart, metadata);
	fputs("\n", metadata);
	free(cantPart);

	fputs("COMPACTION_TIME=", metadata);
	char* compactTime = intToString(compactionTime); //ROMPE
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
		f = fopen(path_aux, "w");
		fputs("SIZE=0", f);
		fputs("\n",f);
		fputs("BLOCKS=[]",f);
		fclose(f);
		free(path_aux);
		free(particion);
	}
	free(path);

}

// ##### FUNCIONES SECUNDARIAS #####

// Convierte un u_int en un string
char* intToString(long a){
	return string_from_format("%ld", a);
}

// Convierte un t_registro* a un t_registro
t_registro convertirAStruct(t_registro *registro){
	t_registro r;
	r.key = registro->key;
	r.value = malloc(strlen(registro->value));
	strcpy(r.value, registro->value);
	r.timeStamp = registro->timeStamp;
	return r;
}
