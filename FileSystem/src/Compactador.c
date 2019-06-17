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
			log_trace(logger, "Hay .tmp para compactar");
			analizarTmpc(path, nombreTabla);
			log_info(logger, "Hay .tmp para compactar");
		}else{
			log_info(logger, "No hay .tmp para compactar");
		}
	}
}

void analizarTmpc(char* path, char* nombreTabla){
	DIR* path_buscado = opendir(path); struct dirent* archivo = readdir(path_buscado);
	char *pathTempc, *line;
	FILE* tempc;
	int particion;
	int caso;

	while(archivo){		//Mientras haya archivos para leer
		if(esArchivoTemporalC(archivo->d_name)){	//Si es un .tmpc
			log_trace(logger, "Entro en %s", archivo->d_name);
			char* lineaTmpc = NULL; size_t lineaTmpc_size = 0;
			pathTempc = string_from_format("%s/%s", path, archivo->d_name);
			tempc = fopen(pathTempc, "r");										//Abri el archivo .tmpc

			while(getline(&lineaTmpc, &lineaTmpc_size, tempc) != -1){				//Mientras existan lineas/renglones en el .tmpc
				log_trace(logger, "Linea: %s", lineaTmpc);
				char** linea = string_split(lineaTmpc, ";"); 						//Dividi el renglon => [TIMESTAMP];[KEY];[VALUE]
				ulong key_buffer = stringToLong(linea[1]);				//KEY
				ulong timestamp_buffer = stringToLong(linea[0]);

				particion = obtenerParticion(nombreTabla, key_buffer); 		//Con la key del .tmpc, busco la particion .bin
				int ultimoBloque = obtenerUltimoBloqueBin(path, particion);	//Obtengo el ultimo bloque para saber si tiene o no
				int size = obtenerSizeBin(path, particion);					//Obtengo el SIZE del .bin de Tablas/nombreTabla/n.bin

				if(ultimoBloque == -1){ 				// No existe un ultimo bloque, por ende, no tengo que comparar Keys
					log_trace(logger,"No hay bloques, agrego uno.");
					int bloqueBin = agregarNuevoBloqueBin();					//Agregar un bloque nuevo
					if(bloqueBin != -1){										//Si existe un bloque libre nuevo
						log_trace(logger, "El nuevo bloque es: %d", bloqueBin);
						char* line = string_from_format("%s;%s;%s",linea[0], linea[1], linea[2]);
						insertarLinea(bloqueBin, line);							//Insertar la linea
						int length = strlen(line)-1;
						free(line);
						actualizarTableBin(nombreTabla, length, particion, bloqueBin, size);	//Actualizar SIZE y la lista de BLOCKS en .bin de /Tables/TablaN/M.bin
					}else{
						log_error(logger, "No hay bloques disponibles para compactar.");
					}
				}else{
					log_trace(logger, "Existen un ultimo bloque: %d.bin", ultimoBloque);
				}

			}
			remove(pathTempc);
		}

		archivo = readdir(path_buscado);
	}
	closedir(path_buscado);
}

char* obtenerBloquesDetabla(FILE* f){

	char* buffer = NULL; size_t buffer_size = 0;
	if(getline(&buffer, &buffer_size, f) != -1){		//SIZE=250
		char* buffer = NULL; size_t buffer_size = 0;
		if(getline(&buffer, &buffer_size, f) != -1){	// BLOCKS=[40,21,82,3]
			char** linea = string_split(buffer, "=");	// [BLOCKS],[40,21,82,3]
			char** aux = string_split(linea[1], "["); 	// 0, 23, 251, 539, 2]
			char** bloquesS = string_split(aux[0], "]");	// 0, 23, 251, 539, 2
			if(bloquesS[0]){
				free(buffer); buffer=NULL;
				free(linea[0]); free(linea[1]);
				free(aux[0]);
				return bloquesS[0];
			}
			free(buffer); buffer=NULL;
			free(linea[0]); free(linea[1]);
			free(aux[0]);
		}
	}

	return NULL;
}

int entraEnUltimoBloque(int size, char* line){
	int lengthLinea = strlen(line);					//Me fijo la longitud de la linea
	log_trace(logger, "(%d + %d) < %d", size, lengthLinea, blockSize);
	if((size + lengthLinea) < blockSize){	//Si la longitud de linea + tamaño de archivo, es menor al tamaño del bloque, es porque entra
		return size + lengthLinea;
	}else{									//No entra en el el bloque
		return -1;
	}
}

int sizeArchivo(FILE* archivo){
	int i = 0;
	while(!feof(archivo)){
		if(fgetc(archivo) != '\n'){
			i++;
		}
	}
	fclose(archivo);
	return i;
}

void actualizarTableBin(char* nombreTabla, int length, int particion, int bloqueBin, int size){
	char* path = string_from_format("%sTable/%s/%d.bin", puntoMontaje, nombreTabla, particion);
	log_trace(logger, "Actualizo sobre: %s", path);
	remove(path);
	FILE* bin = fopen(path, "w+");
	free(path);

	log_trace(logger, "La longitud de la linea es: %d", length);

	char* line = string_from_format("SIZE=%d \n", length+size);
	fputs(line, bin);
	free(line);

	line = string_from_format("BLOCKS=[%d]", bloqueBin);
	fputs(line, bin);
	free(line);

	fclose(bin);
}

void insertarLinea(int bloqueNumero, char* linea){
	char* path = string_from_format("%sBloques/%d.bin", puntoMontaje, bloqueNumero);
	FILE* bloqueBin = fopen(path, "w");
	fputs(linea, bloqueBin);
	fputs("\n",bloqueBin);
	free(path);
	fclose(bloqueBin);
}

int agregarNuevoBloqueBin(){
	for(int i=0;i<blocks;i++){
		if(!bitarray_test_bit(bitarray, i)){ //Si existe un bloque libre
			bitarray_set_bit(bitarray, i);
			return i;
		}
	}
	return -1;
}

bool esArchivoTemporal(char* nombre){
	return (string_starts_with(nombre,"A") && string_ends_with(nombre,".tmp")); 	//Empieza con A y termina con .tmp (A0.tmp, A1.tmp, ...)
}

bool esArchivoTemporalC(char* nombre){
	return (string_starts_with(nombre,"A") && string_ends_with(nombre,".tmpc")); 	//Empieza con A y termina con .tmpc (A0.tmpc, A1.tmpc, ...)
}

int obtenerUltimoBloqueBin(char* path, int particion){	//BLOCKS=[20, 10, 5, 674]
	int ultimoBloque;
	char* pathBin = string_from_format("%s/%d.bin", path, particion);
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
	fclose(bin);
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
		while(blocksFree[i]){				//Busco al ultimo bloque existente.
			ultimoBlock = blocksFree[i];
			i++;
		}
		ultimoBloque = atoi(ultimoBlock);	//Lo transformo a int
		i=0;
		while(blocksFree[i]){				//Libero el char**
			free(blocksFree[i]);
			i++;
		}
		free(blocksFree);
		return ultimoBloque;

	}else{
		return -1;
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

ulong stringToLong(char* strToInt){
	char* endptr, *str = strToInt;
	ulong numero = strtoul(str, &endptr, 10);
	return numero;
}

/*


				if(ultimoBloque != -1){ //Si existe un ultimo bloque, voy a tener que comparar key
					log_trace(logger,"El ultimo bloque es: %d.bin", ultimoBloque);

					ulong timestampBin = tieneKeyIgual(key_buffer, ultimoBloque);
					if(timestampBin != -1){			//Si existe una KEY en el bloque, igual a la del .tmpc, entra
						log_trace(logger, "El bloque %d.bin tiene igual key: %d", ultimoBloque, key_buffer);

						if(timestamp_buffer > timestampBin){ //Si el timestamp del .tmpc es mayor al del .bin, se cambia, sino no.
							log_trace(logger, "Timestamp del .tmpc mayor, cambio");
							line = string_from_format("%s;%s;%s",linea[0], linea[1], linea[2]);	//La linea que voy a ingresar en el archivo Bloques/n.bin
						}else{
							log_trace(logger, "Timestamp del .tmpc menor, no cambio");
						}

					}else{
						line = string_from_format("%s;%s;%s",linea[0], linea[1], linea[2]);	//La linea que voy a ingresar en el archivo Bloques/n.bin
					}
					int entra = entraEnUltimoBloque(size, line);
					if(entra != -1){								//Ver si la frase entra en el ultimo bloque
						log_trace(logger, "La linea entra en el ultimo bloque, la inserto.");
						insertarLinea(ultimoBloque, line);						//Si entra, insertlo la frase
						int length = strlen(line) - 1;
//						actualizarTableBin(nombreTabla, length, particion, bloqueBin);
						free(line);
					}else{
						log_trace(logger,"No entra en el ultimo bloque, agrego otro.");
						int bloqueBin = agregarNuevoBloqueBin();					//Agregar un bloque nuevo
						if(bloqueBin != -1){										//Si existe un bloque libre nuevo
							log_trace(logger, "El nuevo bloque es: %d", bloqueBin);
							char* line = string_from_format("%s;%s;%s",linea[0], linea[1], linea[2]);
							insertarLinea(bloqueBin, line);							//Insertar la linea en el bloque
							int length = strlen(line)-1;
							free(line);
							actualizarTableBin(nombreTabla, entra, particion, bloqueBin);		//Actualizar SIZE y la lista de BLOCKS en .bin de /Tables/TablaN/M.bin
						}else{
							log_error(logger, "No hay bloques disponibles para compactar.");
						}
						//Si no entra, agregar un Bloque Nuevo
						//Insertar la linea

					}

				}else{											//Si no existe un ultimo bloque, por ende, no tengo que comparar Keys


				}

				free(linea[0]);
				free(linea[1]);
				free(linea[2]);
				free(linea);
				free(buffer);
				buffer = NULL;
			}
*/
