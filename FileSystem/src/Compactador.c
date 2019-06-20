#include "LFS.h"

void *compactacion(argumentos_compactacion *args){
	//Tomo los datos del struct por parametro
	char* nombreTabla = malloc(strlen(args->nombreTabla));
	strcpy(nombreTabla, args->nombreTabla);
	int tiempo = args->compactation_time/1000;
	free(args->nombreTabla); free(args);

	while(1){
		sleep(tiempo);
		char* path = string_from_format("%sTable/%s", puntoMontaje, nombreTabla);
		if(renombrarArchivosTemporales(path)){		//Si hay para renombrar, hay para compactar.
			analizarTmpc(path, nombreTabla);
			log_info(logger, "Se realizo la compactacion de la tabla: %s", nombreTabla);
		}else{
			log_info(logger, "No hay .tmp para compactar");
		}
	}
}

void analizarTmpc(char* path, char* nombreTabla){
	DIR* path_buscado = opendir(path); struct dirent* archivo = readdir(path_buscado);
	char *pathTempc;
	FILE* tempc;
	int particion;
	log_info(logger, "Compactacion de la tabla: %s", nombreTabla);
	while(archivo){		//Mientras haya archivos para leer
		if(esArchivoTemporalC(archivo->d_name)){	//Si es un .tmpc
			log_trace(logger, "Entro en %s", archivo->d_name);
			char* lineaTmpc = NULL; size_t lineaTmpc_size = 0;
			pathTempc = string_from_format("%s/%s", path, archivo->d_name);
			tempc = fopen(pathTempc, "r");										//Abri el archivo .tmpc

			while(getline(&lineaTmpc, &lineaTmpc_size, tempc) != -1){				//Mientras existan lineas/renglones en el .tmpc
				log_trace(logger, "Linea: %s", lineaTmpc);
				char** linea = string_split(lineaTmpc, ";"); 						//Dividi el renglon => [TIMESTAMP];[KEY];[VALUE]
				ulong key_tmpc = stringToLong(linea[1]);				//KEY
				ulong timestamp_tmpc = stringToLong(linea[0]);		//TIMESTAMP

				particion = obtenerParticion(nombreTabla, key_tmpc); 		//Con la key del .tmpc, busco la particion .bin
				char* path_bin = string_from_format("%s/%d.bin", path, particion);
				FILE* fBin = fopen(path_bin, "r+");

				int ultimoBloque = obtenerUltimoBloqueBin(fBin);	//Obtengo el ultimo bloque para saber si tiene o no

				log_trace(logger, "El ultimo bloque: %d.bin", ultimoBloque);
				char* bloque_keyRepetida = existeKeyEnBloques(key_tmpc, fBin);	//Verificar si existe la KEY en alguno de los bloques

				if(strcmp(bloque_keyRepetida, "false")){		//Existe la key en algun bloque
					log_trace(logger, "EXISTE la key en el bloque: %s", bloque_keyRepetida);
					//Si existe la key en alguno de los bloques, comparo timestamp
						//Si el timestamp del bloque es mayor, no pasa naranja
						//Si el timestamp del .tmpc es mayor, tengo que reemplazar esa LINEA
				}else{
					log_trace(logger, "NO EXISTE la key en otro bloque");		//Si no existe la KEY en ningun bloque, la agrego al final del ultimo bloque, si entra.
					char* line = string_from_format("%s;%s;%s",linea[0], linea[1], linea[2]);
					free(linea[0]);free(linea[1]);free(linea[2]);free(linea);

					if(entraEnBloque(line, ultimoBloque)){			//Entra en el ultimo bloque
						log_trace(logger, "ENTRA, no es necesario otro bloque");
						insertarLinea(ultimoBloque, line);
						modificarBinTabla(line, NULL, fBin, path_bin);	//Actualizar SIZE del .bin de Tablas
						free(line);
					}else{			//No entra en el ultimo bloque
						log_trace(logger, "NO ENTRA");
						int sizeBloque = calcularTamanoBloque(ultimoBloque);
						int until = 64 - sizeBloque;
						char* newLine = string_substring_until(line, until);		//Inserto hasta donde puedo, el resto va en otro bloque
						log_trace(logger, "Inserto %d caracteres, y %d en otro", until, strlen(line)-until);
						insertarLinea(ultimoBloque, newLine);
						free(newLine);

						char* nuevoBloque = agregarNuevoBloqueBin();
						int newBlock = atoi(nuevoBloque);
						if(nuevoBloque){
							char* path_bloque = string_from_format("%sBloques/%s.bin", puntoMontaje, nuevoBloque);
							FILE* bloque = fopen(path_bloque, "w");
							free(path_bloque);
							char* elseLine = string_substring_from(line, until);
							insertarLinea(newBlock, elseLine);
							free(elseLine);
							fclose(bloque);
							modificarBinTabla(line, nuevoBloque, fBin, path_bin);
							free(nuevoBloque);
						}else{
							log_error(logger, "No existen bloques libres.");
						}
						free(line);
					}

				}
				free(path_bin);
				fclose(fBin);
			}

			remove(pathTempc);
		}

		archivo = readdir(path_buscado);
	}
	closedir(path_buscado);
}

bool crearNuevoBloque(){
	char* nuevoBloque = agregarNuevoBloqueBin();
	if(nuevoBloque){
		char* path_bloque = string_from_format("%sBloques/%s.bin", puntoMontaje, nuevoBloque);
		FILE* bloque = fopen(path_bloque, "w");
		fclose(bloque);
		return true;
	}else{
		log_error(logger, "No existen bloques libres.");
		return false;
	}
}

int calcularTamanoBloque(int numeroBloque){
	char* path = string_from_format("%s/Bloques/%d.bin", puntoMontaje, numeroBloque);
	FILE* f = fopen(path, "r");
	int size = sizeArchivo(f)-1;
	fclose(f);
	free(path);
	return size;
}

void modificarBinTabla(char* linea, char* nuevoBloque, FILE* bin, char* path_bin){
	int length = strlen(linea)-1;
	int size = obtenerSizeBin(bin);
	int size_f = size+length;
	char* size_escritura = string_from_format("SIZE=%d\n", size_f);
	char* lineaBin = NULL; size_t lineaBin_size = 0;

	if(getline(&lineaBin, &lineaBin_size, bin) != -1){		//SIZE=250
		free(lineaBin);
		lineaBin = NULL;
		if(getline(&lineaBin, &lineaBin_size, bin) != -1){}	//char BLOCKS=[40,21,82,3]
	}
	fclose(bin);
	bin = fopen(path_bin, "w+");
	fputs(size_escritura, bin);		//Escribo "SIZE=250"
	free(size_escritura);

	if(nuevoBloque != NULL){		//Si me pasaron un bloque para asignar
		char* bloques_escritura = string_substring_until(lineaBin, strlen(lineaBin)-1);
		free(lineaBin); lineaBin = NULL;
		char* input = string_from_format("%s,%s]", bloques_escritura, nuevoBloque);
		free(bloques_escritura);
		fputs(input, bin);
		log_trace(logger, "Bloques: %s", input);
		free(input);
	}else{
		fputs(lineaBin, bin);
		free(lineaBin);
	}
}

bool entraEnBloque(char* line, int bloque){
	int lengthLinea = strlen(line)-1;					//Me fijo la longitud de la linea
	char* path = string_from_format("%sBloques/%d.bin", puntoMontaje, bloque);
	FILE* f = fopen(path, "r");
	free(path);
	int size = sizeArchivo(f)-1;
	log_trace(logger, "(%d + %d) < %d", size, lengthLinea, blockSize);
	return (size + lengthLinea <= blockSize);	//Si la longitud de linea + tamaño de archivo, es menor al tamaño del bloque, es porque entra
}

char* existeKeyEnBloques(ulong key_tmpc, FILE* binTabla){
	char* lineaBin = NULL; size_t lineaBin_size = 0;
	char* append = NULL;
	if(getline(&lineaBin, &lineaBin_size, binTabla) != -1){		//SIZE=250
		free(lineaBin);
		lineaBin = NULL;
		if(getline(&lineaBin, &lineaBin_size, binTabla) != -1){	//char BLOCKS=[40,21,82,3]
			char** bloques = string_split(lineaBin, "=");	//char [BLOCKS,[40,21,82,3]]
			char** blocks = obtenerBloques(bloques[1]);		//char [40,21,82,3]
			int i=0;
			if(blocks){
				while(blocks[i]){		//Mientras existan bloques
					log_trace(logger, "Entro en el bloque %s.bin", blocks[i]);

					if(existeKey(key_tmpc, blocks[i], append)){	//Existe key en el bloque?
						free(bloques[0]);free(bloques[1]);free(bloques);
						fseek(binTabla, 0, SEEK_SET);
						return blocks[i];
					}
					append = obtenerUltimaLinea(blocks[i]);
					log_trace(logger, "El append es: %s", append);
					i++;
				}
				free(bloques[0]);free(bloques[1]);free(bloques);
			}
		}
	}
	fseek(binTabla, 0, SEEK_SET);
	return "false";
}

char* obtenerUltimaLinea(char* bloque){
	char* path = string_from_format("%sBloques/%s.bin",puntoMontaje, bloque);
	FILE* bloque_bin = fopen(path, "r");
	free(path); int renglon = 0;
	char* buffer_bloque_bin = NULL; size_t size_buffer_bloque_bin = 0;

	while(getline(&buffer_bloque_bin, &size_buffer_bloque_bin, bloque_bin) != -1){
		free(buffer_bloque_bin);
		buffer_bloque_bin = NULL;
		renglon++;
	}
	fseek(bloque_bin, 0, SEEK_SET);
	for(int j=0; j<renglon-1; j++){
		if(getline(&buffer_bloque_bin, &size_buffer_bloque_bin, bloque_bin) != -1){
			free(buffer_bloque_bin);
			buffer_bloque_bin = NULL;
		}
	}
	if(getline(&buffer_bloque_bin, &size_buffer_bloque_bin, bloque_bin) != -1){}

	fclose(bloque_bin);
	return buffer_bloque_bin;
}

bool existeKey(ulong key, char* bloque_numero, char* append){
	char* path = string_from_format("%sBloques/%s.bin",puntoMontaje, bloque_numero);
	FILE* bloque_bin = fopen(path, "r");
	free(path);
	char* buffer_bloque_bin = NULL; size_t size_buffer_bloque_bin = 0;

	if(append){	//Si existe "append", es porque la ultima linea del bloque anterior estaba cortada y tengo que unirla con la primera de este bloque
		if(getline(&buffer_bloque_bin, &size_buffer_bloque_bin, bloque_bin) != -1){
			char* line = string_from_format("%s%s",append, buffer_bloque_bin);
			log_trace(logger, "La linea final queda: %s", line);
			char** linea = string_split(line, ";");
			free(line);
			ulong key_bloque = stringToLong(linea[1]);
			if(key==key_bloque){
				free(linea[0]);free(linea[1]);free(linea[2]);free(linea);
				return true;
			}
			free(linea[0]);free(linea[1]);free(linea[2]);free(linea);
			free(buffer_bloque_bin);
			buffer_bloque_bin = NULL;
		}
	}

	while(getline(&buffer_bloque_bin, &size_buffer_bloque_bin, bloque_bin) != -1){	//[TIMESTAMP];[KEY];[VALUE]
		log_trace(logger, "%s", buffer_bloque_bin);
		char** linea = string_split(buffer_bloque_bin, ";");
		if(!linea[1] || !linea[2]){
			break;
		}
		ulong key_bloque = stringToLong(linea[1]);
		if(key==key_bloque){
			free(linea[0]);free(linea[1]);free(linea[2]);free(linea);
			return true;
		}
		free(linea[0]);free(linea[1]);free(linea[2]);free(linea);
		free(buffer_bloque_bin);
		buffer_bloque_bin = NULL;

	}
	fclose(bloque_bin);
	return false;
}

char** obtenerBloques(char* bloques){		//char [40,21,82,3]
	char** aux1 = string_split(bloques, "["); 		// 0, 23, 251, 539, 2]
	char** aux2 = string_split(aux1[0], "]");		// 0, 23, 251, 539, 2
	free(aux1[0]); free(aux1);
	char** bloquesString = string_split(aux2[0], ",");	// [[0],[23],[251],[539],[2]]
	free(aux2[0]); free(aux2);
	if(bloquesString){	//Si es distinto a "", devolve lo que tengas
		return bloquesString;
	}else{
		return NULL;
	}

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

int sizeArchivo(FILE* archivo){
	int i = 0;
	while(!feof(archivo)){
		if(fgetc(archivo) != '\n'){
			i++;
		}
	}
	fseek(archivo, 0, SEEK_SET);
	return i;
}

void insertarLinea(int bloqueNumero, char* linea){
	char* path = string_from_format("%sBloques/%d.bin", puntoMontaje, bloqueNumero);
	FILE* bloqueBin;
	if(access(path, F_OK) != -1){
		bloqueBin = fopen(path, "r+");
	}else{
		bloqueBin = fopen(path, "w");
	}
	fseek(bloqueBin, 0, SEEK_END);
	fputs(linea, bloqueBin);
	free(path);
	fclose(bloqueBin);
}

char* agregarNuevoBloqueBin(){
	for(int i=0;i<blocks;i++){
		if(!bitarray_test_bit(bitarray, i)){ //Si existe un bloque libre
			bitarray_set_bit(bitarray, i);
			return string_itoa(i);
		}
	}
	return NULL;
}

bool esArchivoTemporal(char* nombre){
	return (string_starts_with(nombre,"A") && string_ends_with(nombre,".tmp")); 	//Empieza con A y termina con .tmp (A0.tmp, A1.tmp, ...)
}

bool esArchivoTemporalC(char* nombre){
	return (string_starts_with(nombre,"A") && string_ends_with(nombre,".tmpc")); 	//Empieza con A y termina con .tmpc (A0.tmpc, A1.tmpc, ...)
}

int obtenerUltimoBloqueBin(FILE* bin){	//BLOCKS=[20, 10, 5, 674]
	int ultimoBloque;
	char* buffer = NULL;
	size_t buffer_size = 0;

	if(getline(&buffer, &buffer_size, bin) != -1){
		free(buffer);
		buffer = NULL;
		if(getline(&buffer, &buffer_size, bin) != -1){
			char** linea = string_split(buffer, "="); // [[BLOCKS], [20, 10, 5, 674]]
			log_trace(logger, "Bloques: %s", linea[1]);
			ultimoBloque = ultimoBloques(linea[1]);	//[20, 10, 5, 674]
			free(linea[0]);
			free(linea[1]);
			free(linea);
			free(buffer);
			buffer = NULL;
		}
	}

	fseek(bin,0,SEEK_SET);
	return ultimoBloque;
}

int obtenerSizeBin(FILE* bin){ // SIZE=1000
	int size;

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
	fseek(bin,0,SEEK_SET);
	return size;
}

int ultimoBloques(char* bloques){ //[0, 23, 251, 539, 2]
	int ultimoBloque; char* ultimoBlock;
	char** aux = string_split(bloques, "["); //0, 23, 251, 539, 2]
	char** bloquesS = string_split(aux[0], "]");	//{0, 23, 251, 539, 2}
	free(aux[0]), free(aux);
//	if(bloquesS[0]){ // Si la lista de bloques existe
	char** blocksFree = string_split(bloquesS[0],",");	//{{0},{23},{251},{539},{2}}
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

//	}else{
//		return -1;
//	}
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

/*				if(ultimoBloque == -1){ 									// No existe un ultimo bloque, por ende, no tengo que comparar Keys
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
						log_error(logger, "No hay bloques disponibles para utilizar.");
					}
				}else{					//Existe un ultimo bloque
*/
