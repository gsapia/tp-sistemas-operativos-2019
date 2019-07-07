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
	char* path_Metadata = string_from_format("%sMetadata/Metadata.bin",puntoMontaje);
	FILE* metadataFS = fopen(path_Metadata,"r");
	free(path_Metadata);
	char* buffer=NULL;
	size_t buffer_size = 0;
	char** linea;
	if(getline(&buffer, &buffer_size, metadataFS) != -1){
		linea = string_split(buffer, "="); 	// [BLOCK_SIZE, 64]
		blockSize = atoi(linea[1]);			// Tamaño de cada bloque
		free(linea[0]);free(linea[1]);free(linea);
		free(buffer);
		buffer = NULL;

		if(getline(&buffer, &buffer_size, metadataFS) != -1){
			linea = string_split(buffer, "="); 	//  [BLOCKS, 5192]
			blocks = atoi(linea[1]);			//Cantidad de Bloques de Metadata
			free(linea[0]);free(linea[1]);free(linea);
			free(buffer);
			buffer = NULL;
		}
	}

	char* path = string_from_format("%sMetadata/Bitmap.bin", puntoMontaje);
	FILE* bitmapFile = fopen(path, "wb");
	truncate(path, blocks/8);
	crearBitmap(path);
	fclose(bitmapFile);
	free(path);
}

void crearBitmap(char* path){
	int fd = open(path, O_RDWR, S_IRUSR | S_IWUSR);
	size_t sizeBitmap = blocks/8;
	bitmap = mmap(NULL, sizeBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //Puntero a un bloque de memoria con el contenido del archivo Bitmap.bin
	bitarray = bitarray_create_with_mode(bitmap, sizeBitmap, LSB_FIRST);

//	munmap(bitmap, sb.st_size);
	close(fd);
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
	crearBloques(path);
	free(path);
}

void crearBloques(char* path){
	FILE* f;
	for(int i=0; i<blocks;i++){
		char* path_aux = string_from_format("%s%d.bin", path,i);
		f = fopen(path_aux, "w");
		truncate(path_aux, blockSize);
		fclose(f);
		free(path_aux);
	}
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
				log_trace(logger, "Salgo de dumplear");
				list_destroy(datosParaDump);
				datosParaDump = NULL;
				flag = 1;
			}else{
				list_destroy(datosParaDump);
			}
		}
		carpeta = readdir(path_buscado);
	}
	list_destroy(memTableAux);
	if(flag==1){
		cantDumps++;
	}
	closedir(path_buscado);
}

//Dumpeo los datos dentro de un archivo ".tmp" que se encuentra dentro de "/Table/carpetaNombre"
void dumpear(t_list *datosParaDump, char* carpetaNombre){
	char** bloques = malloc(sizeof(char)*10);
	char* particionTemp = intToString(cantDumps);

	//Lo escribo
	char* linea = lineasEntera(datosParaDump);
	int cant = strlen(linea)/blockSize;
	char** lineas = dividirLinea(linea);

	for(int i=0; i<(cant+1);i++){
		bloques[i] = getNewBloque();
		char* temp_path = string_from_format("%sBloques/%s.bin", puntoMontaje, bloques[i]);
		FILE* temp = fopen(temp_path, "r+");
		free(temp_path);

		fputs(lineas[i], temp);
		fclose(temp);
	}
	liberarArrayString(lineas);
	crearArchivoTemporal(carpetaNombre, particionTemp, bloques, strlen(linea), cant+1);	//Creo el .tmp con el bloque que pedí y el size
	free(linea);
	free(particionTemp);

}

char** dividirLinea(char* linea){
	int delim = 0;
	int cant = strlen(linea)/blockSize;
	char** lineas = malloc(strlen(linea)*(cant+1));

	for(int i=0; i<cant+1; i++){
		int length_aux = strlen(linea) - blockSize * i;

		if(length_aux > blockSize){		//Osea tiene menos de 64 caracteres
			lineas[i] = string_substring(linea, delim, blockSize);
			delim = delim + blockSize;
		}else{
			if(length_aux < 0){
				lineas[i] = string_substring(linea, delim, -length_aux);
			}else{
				lineas[i] = string_substring(linea, delim, length_aux);
			}
		}
	}
	return lineas;
}

char* lineasEntera(t_list* lista){
	char* linea_return = string_new();
	t_registro *registro;

	while(!list_is_empty(lista)){
		registro = list_remove(lista,0);
		char* timestamp = string_from_format("%llu", registro->timeStamp);
		char* key = intToString(registro->key);
		char* linea_list = string_from_format("%s;%s;%s\n", timestamp, key, registro->value); // [TIMESTAMP];[KEY];[VALUE]
		free(key); free(timestamp);

		string_append(&linea_return, linea_list);

		free(linea_list);
		free(registro->nombre_tabla);
		free(registro->value);
		free(registro);
	}
	return linea_return;
}


void crearArchivoTemporal(char* nombreTabla, char* particionTemp, char** bloques, int size, int cantidadBloques){
	char* path = string_from_format("%sTable/%s/A%s.tmp", puntoMontaje, nombreTabla, particionTemp);
	FILE* f = fopen(path, "w");
	char* auxSize = string_from_format("SIZE=%d", size);
	fputs(auxSize, f);
	free(auxSize);
	fputs("\n",f);

	char* auxBloques;
	char* bloque = string_new();
	string_append(&bloque, bloques[0]);
	log_trace(logger, "Bloque: %s", bloque);
	free(bloques[0]);
	log_trace(logger, "%Cantidad de bloques usados: d");
	for(int i=1;i<cantidadBloques;i++){
		string_append(&bloque,",");
		string_append(&bloque, bloques[i]);
		log_trace(logger, "Bloque: %s", bloque);
	}
	auxBloques = string_from_format("BLOCKS=[%s]", bloque);
	free(bloque);

	fputs(auxBloques,f);

	for(int i=0;i<cantidadBloques;i++){
		free(bloques[i]);
	}
	free(bloques);

	free(auxBloques);
	free(path);
	fclose(f);
}


FILE* obtenerMetaDataLectura(char* nombreTabla){
	char* path = string_from_format("%sTable/%s/Metadata", puntoMontaje, nombreTabla);
	FILE* metadata = fopen(path, "r");
	free(path);
	return metadata;
}


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

FILE* obtenerBIN(int particion, char* nombreTabla){
	char* nombre_bin = intToString(particion);
	char* path = string_from_format("%sTable/%s/%s.bin", puntoMontaje, nombreTabla, nombre_bin);
	FILE *bin = fopen(path, "r+");
	free(path);
	free(nombre_bin);
	return bin;
}

void crearDirectiorioDeTabla(char* nombreTabla){
	char* path = string_from_format("%sTable/%s", puntoMontaje, nombreTabla);
	if(mkdir(path, 0777) != 0){}
	free(path);
}

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

void crearBinDeTabla(char* nombreTabla, int cantParticiones){
	FILE* f;
	char* path = string_from_format("%sTable/%s/", puntoMontaje, nombreTabla);
	char* particion;
	for(int i=0;i<cantParticiones;i++){
		particion = intToString(i);
		char* path_aux = string_from_format("%s%s.bin", path, particion);
		char* numeroBloque = getNewBloque();
		f = fopen(path_aux, "w");
		fputs("SIZE=0  ", f);
		fputs("\n",f);
		char* auxBloques = string_from_format("BLOCKS=[%s]", numeroBloque);
		free(numeroBloque);
		fputs(auxBloques,f);
		fclose(f);
		free(auxBloques);
		free(path_aux);
		free(particion);
	}
	free(path);

}

// ##### FUNCIONES SECUNDARIAS #####

char* intToString(long a){
	return string_from_format("%ld", a);
}

t_registro convertirAStruct(t_registro *registro){
	t_registro r;
	r.key = registro->key;
	r.value = malloc(strlen(registro->value));
	strcpy(r.value, registro->value);
	r.timeStamp = registro->timeStamp;
	return r;
}
