#include "LFS.h"

void *compactacion(argumentos_compactacion *args){
	//Tomo los datos del struct por parametro
	char* nombreTabla = malloc(strlen(args->nombreTabla));
	strcpy(nombreTabla, args->nombreTabla);
	int tiempo = args->compactation_time/1000;
	free(args->nombreTabla); free(args);
	FILE* metadata = obtenerMetaDataLectura(nombreTabla);
	//Saco la cantidad de particiones en el Metadata
	int i = 0;
	size_t buffer_size = 0; char* buffer = NULL;
	while(getline(&buffer, &buffer_size, metadata) != -1 && i<1){
		free(buffer);
		buffer = NULL;
		i++;
	}
	char** linea= string_split(buffer, "=");
	free(buffer);
	int particiones = atoi(linea[1]) + 1;
	free(linea[0]); free(linea[1]); free(linea);

	while(1){
		sleep(tiempo);
		char* path = string_from_format("%sTable/%s", puntoMontaje, nombreTabla);
		if(renombrarArchivosTemporales(path)){		//Si hay para renombrar, hay para compactar.
			analizarTmpc(path, nombreTabla);
			log_info(logger, "Hay .tmp para compactar");
		}else{
			log_info(logger, "No hay .tmp para compactar");
		}
	}
}

bool renombrarArchivosTemporales(char* path){
	char* path_aux, *nuevoNombre; bool flag = false;
	DIR* path_buscado = opendir(path);
	struct dirent* archivo = readdir(path_buscado);

	while(archivo){		//Mientras existan archivos

		if(esArchivoTemporal(archivo->d_name)){
			path_aux = string_from_format("%s/%s", path, archivo->d_name);
			if(access(path_aux, F_OK) != -1){	//Si existe el archivo .tmp y puedo acceder, renombralo a .tmpc / Si no puedo renombrarlo habiendolo acotado, es porque no es un .tmp valido y es fruta
				nuevoNombre = string_from_format("%sc", path_aux);
				rename(path_aux, nuevoNombre);
				free(nuevoNombre);
				flag = true;
			}
			free(path_aux);
		}
		archivo = readdir(path_buscado);
	}
	closedir(path_buscado);
	return flag;
}

void analizarTmpc(char* path, char* nombreTabla){
	DIR* path_buscado = opendir(path);
	struct dirent* archivo = readdir(path_buscado);
	char* pathTempc;
	FILE* tempc;
	int particion;

	while(archivo){		//Mientras haya archivos para leer
		if(esArchivoTemporalC(archivo->d_name)){	//si es un .tmpc
			char* buffer = NULL;
			size_t buffer_size = 0;
			pathTempc = string_from_format("%s/%s", path, archivo->d_name);
			tempc = fopen(pathTempc, "r");

			while(getline(&buffer, &buffer_size, tempc) != -1){ 				//Mientras existan lineas/renglones en el .tmpc
				char** linea = string_split(buffer, ";"); 						//Dividi el renglon => [TIMESTAMP];[KEY];[VALUE]
				char* endptrKey, *keystr = linea[1];
				ulong key_buffer = strtoul(keystr, &endptrKey, 10);
				char* timestampstr = linea[0], *endptrTimestamp;
				ulong timestamp_buffer = strtoul(timestampstr, &endptrTimestamp, 10);
				log_trace(logger,"El value es: %s", linea[2]);
				particion = obtenerParticion(nombreTabla, key_buffer); 		//Con la key del .tmpc, busco la particion .bin
				int ultimoBloque = obtenerUltimoBloqueBin(path, particion);
				int size = obtenerSizeBin(path, particion);
				log_trace(logger,"El size es: %d", size);

				if(ultimoBloque != -1){ //Si existe un ultimo bloque
					log_trace(logger,"El ultimo bloque es: %d", ultimoBloque);
					//Ver si la frase entra en el ultimo bloque
					//Si entra, joya
					//Si no entra, agregar un Bloque Nuevo
					//Insertar la linea
					//Actualizar SIZE en .bin
					//Actualizar BLOCKS en .bin
				}else{
					log_trace(logger,"No hay bloques, agrego uno.");
					//Agregar un bloque nuevo
					//Insertar la linea
					//Actualizar SIZE en .bin
					//Actualizar BLOCKS en .bin
				}

				free(linea[0]);
				free(linea[1]);
				free(linea[2]);
				free(linea);
				free(buffer);
				buffer = NULL;
			}
			remove(pathTempc);
		}

		archivo = readdir(path_buscado);
	}
	closedir(path_buscado);
}

bool esArchivoTemporal(char* nombre){
	return (strcmp(nombre, ".") && strcmp(nombre, "..") && string_starts_with(nombre,"A") && string_ends_with(nombre,".tmp")); 	//Si es distinto a "." y "..", empieza con A y termina con .tmp (A0.tmp, A1.tmp, ...)
}

bool esArchivoTemporalC(char* nombre){
	return (strcmp(nombre, ".") && strcmp(nombre, "..") && string_starts_with(nombre,"A") && string_ends_with(nombre,".tmpc")); 	//Si es distinto a "." y "..", empieza con A y termina con .tmpc (A0.tmpc, A1.tmpc, ...)
}

int obtenerUltimoBloqueBin(char* path, int particion){	//BLOCKS=[20, 10, 5, 674]
	int ultimoBloque;
	char* particionS = intToString(particion);
	char* pathBin = string_from_format("%s/%s.bin", path, particionS);
	free(particionS);
	FILE* bin = fopen(pathBin, "r");
	free(pathBin);

	char* buffer = NULL;
	size_t buffer_size = 0;

	if(getline(&buffer, &buffer_size, bin) != -1){
		free(buffer);
		buffer = NULL;
		if(getline(&buffer, &buffer_size, bin) != -1){
			char** linea = string_split(buffer, "="); // [[BLOCKS], [20, 10, 5, 674]]
			free(linea[0]);
			ultimoBloque = ultimoBloques(linea[1]);	//[20, 10, 5, 674]

			free(linea[1]);
			free(linea);
			free(buffer);
			buffer = NULL;
		}
	}

	fclose(bin);
	return ultimoBloque;
}

int obtenerSizeBin(char* path, int particion){ // SIZE=1000
	int size;
	char* particionS = intToString(particion);
	char* pathBin = string_from_format("%s/%s.bin", path, particionS);
	FILE* bin = fopen(pathBin, "r");

	char* buffer = NULL;
	size_t buffer_size = 0;
	if(getline(&buffer, &buffer_size, bin) != -1){
		char** linea = string_split(buffer, "="); //[[SIZE][1000]]
		size = atoi(linea[1]);
		free(linea[0]);
		free(linea[1]);
		free(linea);
		free(buffer);
		buffer = NULL;
	}
	return size;
}

int ultimoBloques(char* bloques){ //[0, 23, 251, 539, 2]
	int ultimoBloque; char* ultimoBlock;
	char** aux = string_split(bloques, "["); //0, 23, 251, 539, 2]
	char** bloquesS = string_split(aux[0], "]");	//0, 23, 251, 539, 2
	free(aux[0]), free(aux);
	if(bloquesS[0]){ // Si la lista de bloques existe
		char** blocksFree = string_split(bloquesS[0],",");
		free(bloquesS[0]); free(bloquesS);
		int i = 0;
		while(blocksFree[i]){
			ultimoBlock = blocksFree[i];
			i++;
		}
		ultimoBloque = atoi(ultimoBlock);
		i=0;
		while(blocksFree[i]){
			free(blocksFree[i]);
			i++;
		}
		free(blocksFree);
		return ultimoBloque;

	}else{
		return -1;
	}
}
/*
void *compactacion(argumentos_compactacion *args){
	//Tomo los datos del struct por parametro
	char* nombreTabla = malloc(strlen(args->nombreTabla));
	strcpy(nombreTabla, args->nombreTabla);
	int tiempo = args->compactation_time/1000;
	free(args->nombreTabla); free(args);
	FILE* metadata = obtenerMetaDataLectura(nombreTabla);
	//Saco la cantidad de particiones en el Metadata
	int i = 0;
	size_t buffer_size = 0; char* buffer = NULL;
	while(getline(&buffer, &buffer_size, metadata) != -1 && i<1){
		free(buffer);
		buffer = NULL;
		i++;
	}
	char** linea= string_split(buffer, "=");
	free(buffer);
	int particiones = atoi(linea[1]) + 1;
	free(linea[0]); free(linea[1]); free(linea);

	while(1){
		sleep(tiempo);
		char* path = string_from_format("%sTable/%s", puntoMontaje, nombreTabla);
		int diferencia = renombrarArchivosTemporales(path, particiones);
		if(diferencia > 0){
			analizarTmpc(path, nombreTabla, diferencia);
			log_info(logger, "%s realizo compactacion de %d .tmpc", nombreTabla, diferencia);
		}
		free(path);
	}
}

void analizarTmpc(char* path, char* nombre_tabla, int diferencia){
	int i = 0;
	char* pathTempc = string_from_format("%s/A%d.tmpc", path, i);
	FILE* tempc;
	int particion;
	while(diferencia != 0){
		if(access(pathTempc, F_OK) != -1){		//Si podes acceder al .tmpc
			tempc = fopen(pathTempc, "r");						//Abro el .tmpc
			free(pathTempc);
			size_t buffer_size = 0;
			char* buffer = NULL;
			while(getline(&buffer, &buffer_size, tempc) != -1){ 				//Mientras existan lineas/renglones en el .tmpc
				char** linea = string_split(buffer, ";"); 						//Dividi el renglon
				char* endptrKey, *keystr = linea[1];
				ulong key_buffer = strtoul(keystr, &endptrKey, 10);

				char* timestampstr = linea[0], *endptrTimestamp;
				ulong timestamp_buffer = strtoul(timestampstr, &endptrTimestamp, 10);

				particion = obtenerParticion(nombre_tabla, key_buffer); 		//Con la key del .tmpc, busco la particion .bin

				discriminadorDeCasos(path, key_buffer, timestamp_buffer, linea[2], particion);
				free(linea[0]);
				free(linea[1]);
				free(linea[2]);
				free(linea);
				free(buffer);
				buffer = NULL;
			}
			free(buffer);
			fclose(tempc);
			pathTempc = string_from_format("%s/A%d.tmpc", path, i);
			remove(pathTempc);
			diferencia--;
		}
		free(pathTempc);
		i++;
		pathTempc = string_from_format("%s/A%d.tmpc", path, i);
	}
	free(pathTempc);
}

void discriminadorDeCasos(char* path, uint16_t key, uint64_t timestamp, char* value, int particion){
	char* pathBin = string_from_format("%s/%d.bin", path, particion);
	FILE* bin = fopen(pathBin, "r+");
	size_t buffer_size = 0;
	char* buffer = NULL;
	bool encontrado = false;
	int renglon = 0;

	// [TIMESTAMP;KEY;VALUE]
	while(getline(&buffer, &buffer_size, bin) > 0){
		renglon++;
		char **linea = string_split(buffer, ";");
		char* endptrKey, *keystr = linea[1];
		ulong key_buffer = strtoul(keystr, &endptrKey, 10);
		if(key == key_buffer){
			encontrado = true;
			char* timestampstr = linea[0];char* endptrTimestamp;

			ulong timestamp_buffer = strtoul(timestampstr, &endptrTimestamp, 10);

			if(timestamp > timestamp_buffer){ //Si el timestamp del .tempc es mayor al del .bin, reemplazo
				char* timestampC = intToString(timestamp);
				char* keyC = intToString(key);
				char* line = string_from_format("%s;%s;%s", timestampC, keyC, value);
				casoParticular(path, particion, bin, line, renglon);
				log_trace(logger, "Salgo");
				free(timestampC);
				free(keyC);
				free(line);
			}
			free(linea[0]);
			free(linea[1]);
			free(linea[2]);
			free(linea);
			break;
		}
		free(linea[0]);
		free(linea[1]);
		free(linea[2]);
		free(linea);
		free(buffer);
		buffer = NULL;
	}

	if(!encontrado){ //Si no se encontro, lo escribo al final
		char* timestampC = intToString(timestamp);
		char* keyC = intToString(key);
		char* line = string_from_format("%s;%s;%s", timestampC, keyC, value);
		fseek(bin, 0, SEEK_END);

		fputs(line, bin);
		free(timestampC);
		free(keyC);
		free(line);
	}
	free(pathBin);
	free(buffer);
	fclose(bin);
}

void casoParticular(char* path, int particion, FILE* bin, char* line, int renglon){
	char* nameAux = string_from_format("%s/%d.bin", path, bin);
	char* nameAux2 =string_from_format("%s/%AuxiliarTemporal.bin", path, bin);
	rename(nameAux, nameAux2);
	free(nameAux);
	char* pathBin = string_from_format("%s/%d.bin2", path, particion);
	FILE* f = fopen(pathBin, "w");
	int i = 0;

	size_t buffer_size = 80;
	char* buffer = malloc(buffer_size * sizeof(char));
	log_trace(logger, "FLAG_1");
	fseek(bin, 0, SEEK_SET);
	while(getline(&buffer, &buffer_size, bin) != -1){
		if(i==renglon){
			log_trace(logger, "Renglon: %d", renglon);
			fputs(line, f);
		}else{
			log_trace(logger, "FLAG_buffer");
			char **linea = string_split(buffer, ";");
			char* buffer_w = string_from_format("%s;%s%;%s", linea[0], linea[1], linea[2]);
			fputs(buffer_w, f);
			free(buffer_w);
			free(linea);
		}
		i++;
	}

	free(buffer);
	free(pathBin);
	fclose(bin);
	fclose(f);
	remove(nameAux2);
	free(nameAux2);
}
*/
