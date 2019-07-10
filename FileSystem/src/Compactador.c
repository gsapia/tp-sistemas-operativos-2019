#include "LFS.h"
#include "Compactador.h"

void *compactacion(argumentos_compactacion *args){
	//Tomo los datos del struct por parametro
	char* nombreTabla = string_from_format("%s", args->nombreTabla);
	int tiempo = args->compactation_time/1000;
	free(args->nombreTabla); free(args);
	char* path = string_from_format("%sTable/%s", puntoMontaje, nombreTabla);

	sleep(tiempo);
	DIR* directorio = opendir(path);
	while(directorio){

		closedir(directorio);
		if(renombrarArchivosTemporales(path)){		//Si hay para renombrar, hay para compactar.
			uint64_t tiempo_aux = getTimestamp();

			analizarTmpc(path, nombreTabla);
			log_info(logger, "Se realizo la compactacion de la tabla: %s", nombreTabla);

			uint64_t tiempo_posta = getTimestamp();
			tiempo_posta = tiempo_posta - tiempo_aux;
			log_info(logger, "El tiempo transcurrido en la compactacion es %llu milisegundos", tiempo_posta);
		}else{
			log_info(logger, "No hay .tmp para compactar");
		}

		sleep(tiempo);
		directorio = opendir(path);
	}
	free(nombreTabla);
	free(path);
}

void analizarTmpc(char* path_tabla, char* nombreTabla){
	t_list* listaCompactar = list_create();		//Voy a poner todos los datos aca
	DIR* path_buscado = opendir(path_tabla); struct dirent* archivo = readdir(path_buscado);

	while(archivo){
		if(esArchivoValido(archivo->d_name)){
//			log_trace(logger, "%s", archivo->d_name);
			char* path_bin = string_from_format("%s/%s" ,path_tabla, archivo->d_name);
			agregarDatosACompactar(listaCompactar, path_bin);
//			log_trace(logger, "Se agregaron los datos a compactar");
			liberarBloques(path_bin);
//			log_trace(logger, "Se liberaron los bloques");
			if(esArchivoTemporalC(archivo->d_name)){remove(path_bin);}
			free(path_bin);
		}
		archivo = readdir(path_buscado);
	}
	closedir(path_buscado);
//	log_trace(logger, "Se comienzan a escribir los datos");
	escribirDatosLista(listaCompactar, nombreTabla);
//	log_trace(logger, "Se escriben los datos");
}

void escribirDatosLista(t_list* lista, char* nombreTabla){
	t_list* listaFiltradaParticion = NULL;
	filtrarRegistrosConMayorTimestamp(lista);
	FILE* metadata = obtenerMetaDataLectura(nombreTabla);
	int particiones = obtenerParticiones(metadata);
	fclose(metadata);

	for(int j=0;j<particiones;j++){

		bool registroIgualResto(datos_a_compactar* registro){
			int key = stringToLong(registro->key);
			if(j == funcionModulo(key,particiones)){
				return true;
			}else{
				return false;
			}
		}
		listaFiltradaParticion = list_filter(lista, (_Bool (*)(void*))registroIgualResto);
//		log_trace(logger, "## Lista %d ##", j);

		char* linea = lineasEnteraCompactacion(listaFiltradaParticion);
		list_destroy(listaFiltradaParticion);

		if(strcmp(linea,"")){		//Si no existen valores para generar la linea, se devuelve un string vacio
//			log_trace(logger, "Linea completa: %s", linea);

			int cant = strlen(linea)/blockSize;
			int* bloques = malloc(sizeof(int)*cant+1);
			char** lineas = dividirLinea(linea);

			for(int i=0; i<(cant+1);i++){
					bloques[i] = getNewBloque();
//					log_trace(logger, "Obtengo el bloque %d",bloques[i]);
					char* block_path = string_from_format("%sBloques/%d.bin", puntoMontaje, bloques[i]);
					FILE* temp = fopen(block_path, "w");
					truncate(block_path, blockSize);
					free(block_path);

					fputs(lineas[i], temp);
					fclose(temp);
				}

			for(int i=0; i<(cant+1);i++){
				free(lineas[i]);
			}
			free(lineas);

			crearArchivoBin(nombreTabla, j, strlen(linea), bloques, cant+1);
			free(linea);
		}else{
			int* bloques = malloc(sizeof(int));
			bloques[0] = getNewBloque();
//			log_trace(logger, "Obtengo el bloque %d",bloques[0]);
			char* block_path = string_from_format("%sBloques/%d.bin", puntoMontaje, bloques[0]);
			FILE* temp = fopen(block_path, "w");
			truncate(block_path, blockSize);
			free(block_path);
			fclose(temp);
			crearArchivoBin(nombreTabla, j, strlen(linea), bloques, 1);
		}
	}

	datos_a_compactar* aux;
	while(!list_is_empty(lista)){
		aux = list_remove(lista,0);
		free(aux->key);
		free(aux->timestamp);
		free(aux->value);
		free(aux);
	}
	list_destroy(lista);
}

char* lineasEnteraCompactacion(t_list* lista){
	char* linea_return = string_new();
	datos_a_compactar *registro;

	for(int i=0; i<list_size(lista);i++){
		registro = list_get(lista,i);
		char* linea_list = string_from_format("%s;%s;%s\n", registro->timestamp, registro->key, registro->value); // [TIMESTAMP];[KEY];[VALUE]

		string_append(&linea_return, linea_list);
		free(linea_list);

	}
	return linea_return;
}

void crearArchivoBin(char* nombreTabla, int bin, int size, int* bloques, int cantidadBloques){
	char* path_bin = string_from_format("%sTable/%s/%d.bin", puntoMontaje, nombreTabla, bin);
	FILE* bin_file = fopen(path_bin, "w");
	char* auxSize = string_from_format("SIZE=%d", size);
	fputs(auxSize, bin_file);
	free(auxSize);
	fputs("\n",bin_file);

	char* auxBloques;
	char* bloque = string_new();
	char* bloque_selec = string_from_format("%d", bloques[0]);
	string_append(&bloque, bloque_selec);
	free(bloque_selec);
	for(int i=1;i<cantidadBloques;i++){
		string_append(&bloque,",");
		char* bloque_selec = string_from_format("%d", bloques[i]);
		string_append(&bloque, bloque_selec);
		free(bloque_selec);
	}
	auxBloques = string_from_format("BLOCKS=[%s]", bloque);
	free(bloque);

	fputs(auxBloques,bin_file);

	free(bloques);
	free(auxBloques);
	free(path_bin);
	fclose(bin_file);
}

void filtrarRegistrosConMayorTimestamp(t_list* lista){
	datos_a_compactar* registro, *reg_aux, *reg_destroy;
	int i = 0, j = 0;
	bool registro_destroy;

	while(i < list_size(lista)){
		registro_destroy = false;
		registro = list_get(lista, i);
		int key_registro = stringToLong(registro->key);
//		log_trace(logger, "Chequeo registro %s", registro->value);
		j = 0;
		while(j < list_size(lista)){
			reg_aux = list_get(lista, j);
			int key_reg_aux = stringToLong(reg_aux->key);
//			log_trace(logger, "Keys: %d es igual a %d ? ", key_registro, key_reg_aux);
			if(key_reg_aux == key_registro && reg_aux != registro){
//				log_trace(logger, "Encuentro al registro: %s, con la misma key", reg_aux->value);
				uint64_t ts_registro = stringToLongLong(registro->timestamp);
				uint64_t ts_reg_aux = stringToLongLong(reg_aux->timestamp);
//				log_trace(logger, "TS: %llu > %llu ?", ts_registro, ts_reg_aux);
				if(ts_registro > ts_reg_aux){		//Si el timestamp del registro actual es mayor que el timestamp del registro al que lo estoy comparando, borro este ultimo registro
//					log_trace(logger, "registro es mayor a reg_aux");
					free(reg_aux->key);
					free(reg_aux->timestamp);
					free(reg_aux->value);
					reg_destroy = list_remove(lista, j);
					free(reg_destroy);
//					log_trace(logger, "Hago free a todo del reg_aux");
				}else{
//					log_trace(logger, "reg_aux es mayor a registro");
					free(registro->key);
					free(registro->timestamp);
					free(registro->value);
					reg_destroy = list_remove(lista, i);
					free(reg_destroy);
//					log_trace(logger, "Hago free a todo del registro");
					registro_destroy = true;
					break;
				}
			}
			j++;
		}
		if(!registro_destroy){
			i++;
		}
	}

}

void liberarBloques(char* path_bin){
//	log_trace(logger, "Entro a LiberarBloques");
	FILE* archivo = fopen(path_bin, "r");
	char** bloques = obtenerBloquesBin(archivo);
	int i = 0;
	while(bloques[i]){
		freeBloque(bloques[i]);
		i++;
	}
	liberarArrayString(bloques);
//	log_trace(logger, "Salgo");
	fclose(archivo);
}

void agregarDatosACompactar(t_list* lista, char* path){
	FILE* archivo = fopen(path, "r");
	int size_bin = obtenerSizeBin(archivo);
	char* append = NULL;

	if(size_bin != 0){ //Osea, tiene datos
//		log_trace(logger, "Tiene %d datos para leer", size_bin);	//Se fija cuantos datos tiene para leer
		int size_ultimo_bloque = size_bin % blockSize;
//		log_trace(logger, "El ultimo bloque va a tener: %d", size_ultimo_bloque);
		char** bloques = obtenerBloquesBin(archivo);
		int i=0;
		while(bloques[i]){
			if(esUltimoBloque(bloques, i)){
//				log_trace(logger, "No existe un bloque siguiente");
				cargarUltimoBloque(bloques[i], lista, size_ultimo_bloque, append);
			}else{
//				log_trace(logger, "%s no es el ultimo bloque", bloques[i]);
				cargarBloqueALista(bloques[i], lista, append);
				append = obtenerUltimaLinea(bloques[i]);
			}
			i++;
		}
		liberarArrayString(bloques);
	}else{
		//No existen datoS
//		log_trace(logger, "No tiene datos");
	}

	fclose(archivo);

}

void cargarUltimoBloque(char* bloque, t_list* lista, int size_lectura, char* append){
	char* path = string_from_format("%sBloques/%s.bin", puntoMontaje, bloque);
	FILE* block_file = fopen(path, "r+");
	free(path);
	char* buffer = NULL; size_t buffer_size = 0;
	int size_actual=0;
	if(append != NULL){
		if(getline(&buffer, &buffer_size, block_file) != -1){		//Para la primera linea si es que habia algo en el bloque anterior
			size_actual = size_actual + strlen(buffer);
//			log_trace(logger, "size_actual de la primera linea: %d", size_actual);
			char* linea_append = string_from_format("%s%s", append, buffer);
			free(append);
//			log_trace(logger, "Linea APPEND: %s", linea_append);
			char** linea = string_split(linea_append, ";");
			free(linea_append);
			cargarLinea(linea, lista);
			liberarArrayString(linea);
			free(buffer);
			buffer = NULL;
		}
	}

	if(size_actual!=size_lectura){	//Si se llego al final de las lineas para leer con solo leer la primera linea, tengo que salir.
		while(getline(&buffer, &buffer_size, block_file) != -1){
//			log_trace(logger, "Linea: %s", buffer);
			size_actual = size_actual + strlen(buffer);

			if(size_actual < size_lectura){
//				log_trace(logger, "%d es menor a %d", size_actual, size_lectura);
				char** linea = string_split(buffer, ";");
				cargarLinea(linea, lista);
				liberarArrayString(linea);
				free(buffer);
				buffer = NULL;
			}else if(size_actual == size_lectura){
//				log_trace(logger, "%d es igual a %d, salgo del while", size_actual, size_lectura);
				char** linea = string_split(buffer, ";");
				cargarLinea(linea, lista);
				liberarArrayString(linea);
				free(buffer);
				buffer = NULL;
				break;
			}
		}
		if(buffer){free(buffer);}
	}else{
//		log_trace(logger, "%d es igual a %d", size_actual, size_lectura);
	}
	fclose(block_file);
}

void cargarBloqueALista(char* bloque, t_list* lista, char* append){
	char* path = string_from_format("%sBloques/%s.bin", puntoMontaje, bloque);
	FILE* block_file = fopen(path, "r+");
	free(path);
	char* buffer = NULL; size_t buffer_size = 0;
	if(append != NULL){
		if(getline(&buffer, &buffer_size, block_file) != -1){		//Para la primera linea si es que habia algo en el bloque anterior
			char* linea_append = string_from_format("%s%s", append, buffer);
			free(append);
//			log_trace(logger, "Linea APPEND: %s", linea_append);
			char** linea = string_split(linea_append, ";");
			free(linea_append);
			cargarLinea(linea, lista);
			liberarArrayString(linea);
			free(buffer);
			buffer = NULL;
		}
	}
	int i = 1;
	while(getline(&buffer, &buffer_size, block_file) != -1){	// [TIMESTAMP;KEY;VALUE]
//		log_trace(logger, "Linea %d: %s", i, buffer);
		i++;
		char** linea = string_split(buffer, ";");
		int len = ftell(block_file);
		if(!esUltimaLinea(block_file)){
			fseek(block_file, len, SEEK_SET);
//			log_trace(logger, "Cargo %s", buffer);
			cargarLinea(linea, lista);
		}
		liberarArrayString(linea);
		free(buffer);
		buffer = NULL;
	}
	if(buffer){free(buffer);}
	buffer = NULL;

	fclose(block_file);
}

bool esUltimaLinea(FILE* f){
	char* buffer = NULL; size_t buffer_size = 0;
	if(getline(&buffer, &buffer_size, f) != -1){
		free(buffer);
		return false;
	}else{
		free(buffer);
		return true;
	}
}

void cargarLinea(char** linea, t_list* lista){
	datos_a_compactar* registro = malloc(sizeof(datos_a_compactar));
	registro->timestamp = string_from_format("%s", linea[0]);
	registro->key = string_from_format("%s", linea[1]);
	char* value_aux = string_from_format("%s", linea[2]);
	registro->value = string_substring(value_aux, 0, strlen(value_aux)-1);
	free(value_aux);
	list_add(lista, registro);
//	log_trace(logger, "Agrego [%s;%s;%s]", registro->timestamp,registro->key,registro->value);
}

bool esUltimoBloque(char** bloques, int indice){
	if(bloques[indice+1]){
		return false;
	}else{
		return true;
	}
}

char** obtenerBloquesBin(FILE* f){
	char* buffer = NULL;
	size_t size_buffer_bloque_bin = 0;
	fseek(f,0,SEEK_SET);
	if(getline(&buffer, &size_buffer_bloque_bin, f) != -1){
		free(buffer); buffer=NULL;
		if(getline(&buffer, &size_buffer_bloque_bin, f) != -1){	//BLOCKS=[41,23,35,12]
			char** blocks_bin= string_split(buffer, "=");
			char* block_aux1 = string_substring(blocks_bin[1], 1, strlen(blocks_bin[1])-2);	// 41,23,35,12
//			log_trace(logger, "Los bloques quedan: %s", block_aux1);
			char** bloques_bin= string_split(block_aux1, ",");
			liberarArrayString(blocks_bin);
			free(block_aux1);
			free(buffer);
			return bloques_bin;
		}
	}
	return NULL;
}

int obtenerSizeBin(FILE* f){
	char* buffer = NULL;
	size_t size_buffer_bloque_bin = 0;
	int size;
	if(getline(&buffer, &size_buffer_bloque_bin, f) != -1){		// SIZE=1243
		char** sizeString= string_split(buffer, "=");
		size = stringToLong(sizeString[1]);
		liberarArrayString(sizeString);
	}
	free(buffer); buffer=NULL;
	return size;
}

bool esArchivoValido(char* nombre_archivo){
	return strcmp(nombre_archivo,".") && strcmp(nombre_archivo,"..") && strcmp(nombre_archivo, "Metadata");
}

bool crearNuevoBloque(){
	int nuevoBloque = getNewBloque();
	if(nuevoBloque){
		char* path_bloque = string_from_format("%sBloques/%d.bin", puntoMontaje, nuevoBloque);
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
		if(getline(&lineaBin, &lineaBin_size, bin) != -1){}	//char* BLOCKS=[40,21,82,3]
	}
	fclose(bin);
	bin = fopen(path_bin, "w+");
	fputs(size_escritura, bin);		//Escribo "SIZE=250"
	free(size_escritura);

	if(nuevoBloque != NULL){		//Si me pasaron un bloque para asignar
		char* bloques_escritura = string_substring_until(lineaBin, strlen(lineaBin)-1);
		char* input = string_from_format("%s,%s]", bloques_escritura, nuevoBloque);
		free(bloques_escritura);
		fputs(input, bin);
		log_trace(logger, "Bloques: %s", input);
		free(input);
		log_trace(logger, "free(input)");
	}else{
		fputs(lineaBin, bin);
	}
	free(lineaBin); lineaBin = NULL;
	log_trace(logger, "free(lineaBin); lineaBin = NULL;");
	fclose(bin);
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

char* existeKey(u_int16_t key, char** bloques){
	int i = 0;
	while(bloques[i]){
		char* path = string_from_format("%sBloques/%s.bin",puntoMontaje, bloques[i]);
		FILE* bloque_bin = fopen(path, "r");
		free(path);

		char* buffer_bloque_bin = NULL; size_t size_buffer_bloque_bin = 0; 	// Preparo datos para el buffer

		while(getline(&buffer_bloque_bin, &size_buffer_bloque_bin, bloque_bin) != -1){	//Dame la linea del archivo

			if(feof(bloque_bin) && bloques[i+1]){	//Si quedo el puntero del archivo en el final, y existe otro bloque siguiente, es porque el resto de la linea esta en ese otro bloque
				char* fstLine = obtenerPrimeraLinea(bloques[i+1]);
				char* linea = string_from_format("%s%s", buffer_bloque_bin, fstLine);		//[TIMESTAMP;KEY;VALUE]
				log_trace(logger, "Leo linea append: %s", linea);
				free(fstLine); free(buffer_bloque_bin); buffer_bloque_bin = NULL;		//Libero los char*
				char** line = string_split(linea, ";");
				free(linea);
				ulong key_bloque = stringToLong(line[1]);
				if(key==key_bloque){
					return string_from_format("%s", bloques[i]);
				}
				free(line[0]);free(line[1]);free(line[2]);free(line);

			}else{												//Si no es fin de archivo, o no existe un proximo bloque, es porque esa linea es valida
				log_trace(logger, "Leo %s", buffer_bloque_bin);
				char** line = string_split(buffer_bloque_bin, ";");	//[TIMESTAMP,KEY,VALUE]
				free(buffer_bloque_bin); buffer_bloque_bin = NULL;
				if(line[1] && line[2]){		//Es el caso en que la primera linea de un bloque esta cortada, y no me sirve que solo tenga uno o dos valores
					log_trace(logger, "Linea[1] y linea[2] existen");
					ulong key_bloque = stringToLong(line[1]);
					if(key==key_bloque){
						log_trace(logger, "%d es igual a %d", key_bloque, key);
						free(line[0]);free(line[1]);free(line[2]);free(line);
						fclose(bloque_bin);
						return string_from_format("%s", bloques[i]);
					}
					log_trace(logger, "La key %d NO es igual a la key %d", key, key_bloque);
				}else{
					log_trace(logger, "Linea[1] o linea[2] NO existen, leo la siguiente");
				}
				int j=0;
				while(line[j]){
					free(line[j]);
					j++;
				}
				free(line);
			}
		}
		free(buffer_bloque_bin);
		fclose(bloque_bin);
		i++;
	}
	return NULL;
}

char* existeKeyEnBloques(uint16_t key_tmpc, FILE* binTabla){
	char* lineaBin = NULL; size_t lineaBin_size = 0;
	char* bloque_keyExistente;
	if(getline(&lineaBin, &lineaBin_size, binTabla) != -1){		//SIZE=250
		free(lineaBin);
		lineaBin = NULL;
		if(getline(&lineaBin, &lineaBin_size, binTabla) != -1){	//char BLOCKS=[40,21,82,3]
			char** bloques = string_split(lineaBin, "=");	//char [BLOCKS,[40,21,82,3]]
			char** blocks = obtenerBloques(bloques[1]);		//char [40,21,82,3]
			bloque_keyExistente = existeKey(key_tmpc, blocks);
			liberarArrayString(blocks);
			liberarArrayString(bloques);
			free(lineaBin); lineaBin=NULL;
		}
	}
	fseek(binTabla, 0, SEEK_SET);

	if(bloque_keyExistente){
		return bloque_keyExistente;
	}else{
		return "false";
	}
}

void liberarArrayString(char** array){
	int i = 0;
	while(array[i]){
			free(array[i]);
			i++;
		}
		free(array);
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
	fseek(archivo, 0, SEEK_END);
	int size = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	return size;
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

int getNewBloque(){
	for(int i=0;i<blocks;i++){
		if(!bitarray_test_bit(bitarray, i)){ //Si existe un bloque libre
			bitarray_set_bit(bitarray, i);
			return i;
		}
	}
	return NULL;
}

void freeBloque(char* bloque){
	int bit = stringToLong(bloque);
//	log_trace(logger, "Libero el bloque %d", bit);
	bitarray_clean_bit(bitarray,bit);
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

uint64_t stringToLong(char* strToInt){
	char* endptr, *str = strToInt;
	uint64_t numero = strtoul(str, &endptr, 10);
	return numero;
}

uint64_t stringToLongLong(char* strToInt){
	char* endptr, *str = strToInt;
	uint64_t numero = strtoull(str, &endptr, 10);
	return numero;
}

/*
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
				uint16_t key_tmpc = stringToLong(linea[1]);				//KEY
				uint64_t timestamp_tmpc = stringToLong(linea[0]);		//TIMESTAMP

				particion = obtenerParticion(nombreTabla, key_tmpc); 		//Con la key del .tmpc, busco la particion .bin
				char* path_bin = string_from_format("%s/%d.bin", path, particion);
				FILE* fBin = fopen(path_bin, "r+");

				int ultimoBloque = obtenerUltimoBloqueBin(fBin);	//Obtengo el ultimo bloque para saber si tiene o no

				log_trace(logger, "El ultimo bloque: %d.bin", ultimoBloque);
				char* bloque_keyRepetida = existeKeyEnBloques(key_tmpc, fBin);	//Verificar si existe la KEY en alguno de los bloques

				if(strcmp(bloque_keyRepetida, "false")){		//Existe la key en algun bloque
					log_trace(logger, "EXISTE la key en el bloque: %s", bloque_keyRepetida);
					free(bloque_keyRepetida);
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
						log_trace(logger, "Termino de modificar la Tabla");
						free(line);
					}else{			//No entra en el ultimo bloque
						log_trace(logger, "NO ENTRA");
						int sizeBloque = calcularTamanoBloque(ultimoBloque);
						int until = blockSize - sizeBloque;
						char* newLine = string_substring_until(line, until);		//Inserto hasta donde puedo, el resto va en otro bloque
						log_trace(logger, "Inserto %d caracteres, y %d en otro", until, strlen(line)-until);
						insertarLinea(ultimoBloque, newLine);
						free(newLine);

						char* nuevoBloque = getNewBloque();
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
							log_trace(logger, "Salgo de modificarBinTabla");
							free(nuevoBloque);
						}else{
							log_error(logger, "No existen bloques libres.");
						}
						log_trace(logger, "Antes del free(line)");
						free(line);
					}
				}
				free(path_bin);
				log_trace(logger, "free(path_bin);");
			}
			free(lineaTmpc);
			remove(pathTempc);
			free(pathTempc);
		}

		archivo = readdir(path_buscado);
	}
	closedir(path_buscado);

*/
