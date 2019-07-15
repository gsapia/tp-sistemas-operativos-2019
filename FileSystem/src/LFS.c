#include "LFS.h"

void* consola(){
	char *linea;
	char *resultado;
	while(1) {
		linea = readline(">");

		if(!strcmp(linea,"exit")){
			free(linea);
			break;
		}
		resultado = apiLissandra(linea);
		free(linea);
		puts(resultado);
		free(resultado);
	}
	log_info(logger, "Se ha cerrado la Consola");
}

char *apiLissandra(char* mensaje){
	char** comando = string_split(mensaje, " ");
	if(comando[0]){
		u_int16_t cantArgumentos = 0;
		while(comando[cantArgumentos+1]){
			cantArgumentos++;
		}

		if(!strcmp(comando[0],"SELECT")){
			//SELECT [NOMBRE_TABLA] [KEY]
			//SELECT TABLA1 3
			free(comando[0]);
			if(cantArgumentos == 2){
				char* nombreTabla = comando[1];
				char* keystr = comando[2];
				char* endptr;
				ulong key = strtoul(keystr, &endptr, 10);
				if(*endptr == '\0'&& key < 65536){
					struct_select_respuesta resultado = selects(nombreTabla, key);
					free(nombreTabla);
					free(keystr);
					free(comando);
					char* valorS;
					if(string_ends_with(resultado.valor, "\n")){
						valorS = string_substring(resultado.valor,0, strlen(resultado.valor)-1);
					}else{
						valorS = string_from_format("%s", resultado.valor);
					}
					free(resultado.valor);
					switch (resultado.estado){
						case ESTADO_SELECT_OK:
							log_trace(logger, "Timestamp: %llu",resultado.timestamp);
							return valorS;
						case ESTADO_SELECT_ERROR_TABLA:
							return strdup("ERROR: La tabla solicitada no existe.");
						case ESTADO_SELECT_ERROR_KEY:
							return strdup("ERROR: Esa tabla no contiene ningun registro con la clave solicitada.");
						default:
							return strdup("ERROR: Ocurrio un error desconocido.");
					}
				}
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: SELECT [NOMBRE_TABLA] [KEY]");
		}
		else if(!strcmp(comando[0],"INSERT")){
			//INSERT [NOMBRE_TABLA] [KEY] “[VALUE]” [Timestamp]
			//INSERT TABLA1 3 "Mi nombre es Lissandra"
			//INSERT TABLA1 3 "Mi nombre es Lissandra" 1548421507

			free(comando[0]);
			if (cantArgumentos > 2){
				char** argumentos = string_n_split(mensaje, 4, " ");
				char** ultimoArgumento = string_split(argumentos[3], "\""); //Sirve para el value

				free(argumentos[0]);
				free(argumentos[1]);
				free(argumentos[2]);
				free(argumentos[3]);
				free(argumentos);

				if(!ultimoArgumento[1]){ // Si viene SIN TIMESTAMP
					char* nombreTabla = comando[1];
					char* keystr = comando[2];
					char* endptr;
					ulong key = strtoul(keystr, &endptr, 10);
					char* valor = ultimoArgumento[0];
					uint64_t timestamp = getTimestamp();

					if (*endptr == '\0' && key < 65536) {
						enum estados_insert resultado = insert(nombreTabla, key, valor, timestamp);
						while(cantArgumentos){
							free(comando[cantArgumentos]);
							cantArgumentos--;
						}
						free(valor);
						free(ultimoArgumento[1]);
						free(ultimoArgumento);
						free(comando);
						switch(resultado){
						case ESTADO_INSERT_OK:
							return strdup("Se realizo INSERT");
						case ESTADO_INSERT_ERROR_TABLA:
							return strdup("ERROR: No existe tabla");
						case ESTADO_INSERT_ERROR_OTRO:
							return strdup("Ocurrio un error desconocido");
						}
					}
				}else{ // Si viene CON TIMESTAMP
					char* nombreTabla = comando[1];
					char* keystr = comando[2];
					char* endptr;
					ulong key = strtoul(keystr, &endptr, 10);
					char* valor = ultimoArgumento[0];
					char* endptr2;
					uint64_t timestamp = strtoull(ultimoArgumento[1], &endptr2, 10);
					if (*endptr == '\0' && key < 65536) {
						enum estados_insert resultado = insert(nombreTabla, key, valor, timestamp);
						while(cantArgumentos){
							free(comando[cantArgumentos]);
							cantArgumentos--;
						}
						free(valor);
						free(ultimoArgumento);
						free(comando);
						switch(resultado){
						case ESTADO_INSERT_OK:
							return strdup("Se realizo INSERT");
						case ESTADO_INSERT_ERROR_TABLA:
							return strdup("ERROR: No existe tabla");
						case ESTADO_INSERT_ERROR_OTRO:
							return strdup("Ocurrio un error desconocido");
						}
					}
				}
				for(int i = 0; ultimoArgumento[i]; i++){
					free(ultimoArgumento[i]);
				}
				free(ultimoArgumento);
			}

			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: INSERT [NOMBRE_TABLA] [KEY] “[VALUE]” TIMESTAMP* ");
		}
		else if(!strcmp(comando[0],"CREATE")){
			//CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
			//CREATE TABLA1 SC 4 60000

			free(comando[0]);
			if(cantArgumentos == 4){
				char* nombreTabla = comando[1];
				char* tipoConsistencia = comando[2];
				char* cantidadParticionesstr = comando[3];
				char* compactionTimestr = comando[4];
				char* endptr = 0;
				ulong cantidadParticiones = strtoul(cantidadParticionesstr, &endptr, 10);
				ulong compactionTime;
				if(*endptr == '\0')
					compactionTime = strtoul(compactionTimestr, &endptr, 10);
				if(*endptr == '\0'){
					// Faltaria revisar si el tipo de consistencia es valido ^
					uint16_t resultado = create(nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
					free(nombreTabla);
					free(tipoConsistencia);
					free(cantidadParticionesstr);
					free(compactionTimestr);
					free(comando);


					switch(resultado){
						case ESTADO_CREATE_OK:
							return strdup("OK: Se creo la carpeta solicitada");
						case ESTADO_CREATE_ERROR_TABLAEXISTENTE:
							return strdup("ERROR: La tabla solicitada ya existe.");
						default:
							return strdup("ERROR: Ocurrio un error desconocido.");
					}
				}
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]");
		}
		else if(!strcmp(comando[0],"DESCRIBE")){
			//DESCRIBE [NOMBRE_TABLA]
			//DESCRIBE TABLA1
			//DESCRIBE

			free(comando[0]);
			if(cantArgumentos == 1){ //	DESCRIBE TABLA
				char* nombreTabla = comando[1];
				struct_describe_respuesta resultado = describe(nombreTabla);;
				free(nombreTabla);
				free(comando);
				switch (resultado.estado){
					case ESTADO_DESCRIBE_OK:
						log_trace(logger,"La consistencia es: %d", resultado.consistencia);
						log_trace(logger,"Las particiones son: %d", resultado.particiones);
						log_trace(logger,"El tiempo de compactacion es: %d", resultado.tiempo_compactacion);
						return strdup("Se hizo el DESCRIBE");
					case ESTADO_DESCRIBE_ERROR_TABLA:
						return strdup("ERROR: La tabla solicitada no existe.");
					default:
						return strdup("ERROR: Ocurrio un error desconocido.");
				}
			} else if(cantArgumentos == 0){ //DESCRIBE

				struct_describe_global_respuesta resultado = describe_global();

				switch(resultado.estado){
					case ESTADO_DESCRIBE_OK:
						return strdup("Se hizo el DESCRIBE");
					case ESTADO_DESCRIBE_ERROR_TABLA:
						return strdup("ERROR: La tabla solicitada no existe.");
					default:
						return strdup("ERROR: Ocurrio un error desconocido.");
				}
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: DESCRIBE [NOMBRE_TABLA]");
		}
		else if(!strcmp(comando[0],"DROP")){
			//DROP [NOMBRE_TABLA]
			//DROP TABLA1

			free(comando[0]);
			if(cantArgumentos == 1){
				char* nombreTabla = comando[1];
				enum estados_drop resultado = drop(nombreTabla);
				free(nombreTabla);
				free(comando);
				switch(resultado){
				case ESTADO_DROP_OK:
					return strdup("Se realizo el DROP");
				case ESTADO_DROP_ERROR_TABLA: // No existe la tabla
					return strdup("ERROR: La tabla solicitada no existe.");
				case ESTADO_DROP_ERROR_OTRO:
					return strdup("ERROR: Ocurrio un error desconocido.");
				}
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: DROP [NOMBRE_TABLA]");
		}

		while(cantArgumentos){
			free(comando[cantArgumentos]);
			cantArgumentos--;
		}
		free(comando[0]);
	}
	free(comando);
	return string_from_format("Comando invalido");
}

struct_select_respuesta selects(char* nombreTabla, u_int16_t key){
	struct_select_respuesta select_respuesta;
	t_registro *aux_memtable;
	t_list *listaFiltro = list_create();
	t_list *listaFiltroMemtable = NULL;
	datos_a_compactar *aux;

	if(existeTabla(nombreTabla)){
		pthread_mutex_t *mutexAux = dictionary_get(diccionario, nombreTabla);
		pthread_mutex_lock(mutexAux);

		bool registro_IgualKey(t_registro *registro){return registro->key == key && !strcmp(registro->nombre_tabla,nombreTabla);}

		agregarRegDeBloquesYTemps(listaFiltro, nombreTabla, key);
//		log_trace(logger, "Agrego reg a bloques, sizeof listaFiltro es: %d", list_size(listaFiltro));
		if(!list_is_empty(memTable)){
//			log_trace(logger, "Memtable tiene registros");
			listaFiltroMemtable = list_filter(memTable, (_Bool (*)(void*))registro_IgualKey);
			if(!list_is_empty(listaFiltroMemtable)){
//				log_trace(logger, "Filtre la memtable, tiene %d registros", list_size(listaFiltroMemtable));
				list_sort(listaFiltroMemtable, (_Bool (*)(void*,void*))ordenarDeMayorAMenorTimestamp);
//				log_trace(logger, "Sort la memtable");
				aux_memtable = list_get(listaFiltroMemtable, 0);
//				log_trace(logger, "Obtuve el primer registro");
				aux = convertirAStructDAC(aux_memtable);
//				log_trace(logger, "Lo Converti en DAC");
				list_add(listaFiltro, aux);
//				log_trace(logger, "Agrego el registro con value: %s", aux->value);
				list_destroy(listaFiltroMemtable);
			}
		}else{
//			log_trace(logger, "Memtable vacia");
		}
		if(!list_is_empty(listaFiltro)){
//			log_trace(logger, "sizeof listaFiltro es: %d");
			list_sort(listaFiltro, (_Bool (*)(void*,void*))ordenarDeMayorAMenorTimestampFinal);
			aux = list_get(listaFiltro, 0);
			select_respuesta = convertirARespuestaSelect(aux);
			datos_a_compactar* aux1;
			while(!list_is_empty(listaFiltro)){
				aux1 = list_remove(listaFiltro, 0);
				free(aux1->key);
				free(aux1->timestamp);
				free(aux1->value);
				free(aux1);
			}
			list_destroy(listaFiltro);
		}else{
			select_respuesta.estado = ESTADO_SELECT_ERROR_KEY;
		}
		log_debug(logger, "SELECT: Recibi Tabla: %s Key: %d", nombreTabla, key);
		pthread_mutex_unlock(mutexAux);
		return select_respuesta;
	}else{
		select_respuesta.estado = ESTADO_SELECT_ERROR_TABLA;
		log_debug(logger, "No existe en el File System la tabla: %s",nombreTabla);
		return select_respuesta;
	}

}

enum estados_insert insert(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp){

	if(existeTabla(nombreTabla)){
		if(sizeof(timeStamp) <= tamValue){
			agregarAMemTable(nombreTabla, key, valor, timeStamp);
			log_debug(logger, "INSERT: Tabla: %s, Key: %d, Valor: %s, Timestamp: %llu", nombreTabla, key, valor, timeStamp);
			return ESTADO_INSERT_OK;
		}else{
			return ESTADO_INSERT_ERROR_OTRO;
		}
	}else{
		log_debug(logger, "No existe en el File System la tabla: %s",nombreTabla);
		return ESTADO_INSERT_ERROR_TABLA;
	}
}

uint16_t create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
//	CREATE TABLA1 SC 4 60000
	uint16_t estado;
	if(!existeTabla(nombreTabla)){
		t_hiloCompactacion* hiloC = malloc(sizeof(t_hiloCompactacion));
		hiloC->nombreTabla = string_from_format("%s", nombreTabla);

		crearDirectiorioDeTabla(nombreTabla);
		crearMetadataDeTabla(nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
		crearBinDeTabla(nombreTabla, cantidadParticiones);

		argumentos_compactacion *args = malloc(sizeof(argumentos_compactacion));
		args->compactation_time = compactionTime;
		args->nombreTabla = malloc(strlen(nombreTabla)+1);
		strcpy(args->nombreTabla, nombreTabla);

		//Creo el mutex

		pthread_mutex_t * mutex = malloc(sizeof(pthread_mutex_t));
		if(pthread_mutex_init(mutex, NULL) != 0){
			log_error(logger, "Mutex de tabla %s: Error - pthread_mutex_init()", nombreTabla);
		}

		dictionary_put(diccionario, nombreTabla, mutex);

		//Creo el hilo
		pthread_attr_t attr;
		pthread_t hiloCompactacion;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if(pthread_create(&hiloCompactacion, &attr, compactacion, args)){
			log_error(logger, "Hilo compactacion: Error - pthread_create()");
			exit(EXIT_FAILURE);
		}

		pthread_attr_destroy(&attr);

		log_debug(logger, "CREATE: Recibi Tabla: %s TipoDeConsistencia: %s CantidadDeParticines: %d TiempoDeCompactacion: %d", nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
		estado = ESTADO_CREATE_OK;
	}else{
		log_info(logger, "La tabla %s ya existe en el FS", nombreTabla);
		estado = ESTADO_CREATE_ERROR_TABLAEXISTENTE;
	}
	return estado;
}

struct_describe_respuesta describe(char* nombreTabla){
	struct_describe_respuesta describe_respuesta;
	if(existeTabla(nombreTabla)){
		FILE* metadata = obtenerMetaDataLectura(nombreTabla);
		char** valores = malloc(4);
		int i = 0;
		size_t buffer_size = 0;
		char* buffer = NULL;

		while(getline(&buffer, &buffer_size, metadata) != -1){ //ALGO=VALOR
			char** linea= string_split(buffer, "=");
			valores[i]=malloc(strlen(linea[1]));
			strcpy(valores[i],linea[1]);
			free(linea[0]);
			free(linea[1]);
			free(linea);
			i++;
			free(buffer);
			buffer = NULL;
		}

		describe_respuesta = convertirARespuestaDescribe(valores[0], valores[1], valores[2]);

		free(valores[0]);free(valores[1]);free(valores[2]);free(valores);
		free(buffer);
		fclose(metadata);
		log_debug(logger, "DESCRIBE: Recibi Tabla: %s", nombreTabla);
		return describe_respuesta;

	}else{
		describe_respuesta.estado = ESTADO_DESCRIBE_ERROR_TABLA;
		log_trace(logger, "No existe la tabla");
		return describe_respuesta;
	}
}

struct_describe_global_respuesta describe_global(){
	struct_describe_global_respuesta respuesta;
	respuesta.estado = ESTADO_DESCRIBE_OK;
	respuesta.describes = dictionary_create();
	struct_describe_respuesta* tabla;
	struct_describe_respuesta aux;
	char* path = string_from_format("%sTable/", puntoMontaje);
	DIR* path_buscado = opendir(path);
	free(path);
	struct dirent* carpeta = readdir(path_buscado);

	if(carpeta){
		while(carpeta){
			if(strcmp(carpeta->d_name, ".") && strcmp(carpeta->d_name, "..")){
				log_trace(logger, "Entro a %s", carpeta->d_name);
				aux = describe(carpeta->d_name);
				tabla = convertirAPuntero(aux);
				dictionary_put(respuesta.describes, carpeta->d_name, tabla);
			}
			carpeta = readdir(path_buscado);
		}
	}else{
		respuesta.estado = ESTADO_DESCRIBE_ERROR_OTRO;
	}
	closedir(path_buscado);
	return respuesta;
}

enum estados_drop drop(char* nombreTabla){
	if(existeTabla(nombreTabla)){	//Tengo certeza que existe la tabla, entonces va a estar en la lista
		pthread_mutex_t *mutexAux = dictionary_remove(diccionario, nombreTabla);
		pthread_mutex_lock(mutexAux);
		char* path = string_from_format("%sTable/%s/", puntoMontaje, nombreTabla);

		bool registro_IgualNombreTabla(t_hiloCompactacion *registro){return !strcmp(registro->nombreTabla,nombreTabla);}

		borrarTabla(path);
//		log_trace(logger, "Antes de borrar el path");
		rmdir(path);
		free(path);
		log_trace(logger, "Borro path");

		log_debug(logger, "DROP: Recibi Tabla: %s", nombreTabla);
		pthread_mutex_unlock(mutexAux);
		log_trace(logger, "mutex_unloc");
		free(mutexAux);
		log_trace(logger, "free()");
		return ESTADO_DROP_OK;
	}else{
		log_debug(logger, "DROP: Recibi Tabla: %s", nombreTabla);
		log_error(logger, "No existe la tabla: %s", nombreTabla);
		return ESTADO_DROP_ERROR_TABLA;
	}
}

datos_a_compactar* convertirAStructDAC(t_registro *aux_memtable){
	datos_a_compactar* registro = malloc(sizeof(datos_a_compactar));
	registro->key = string_from_format("%d",aux_memtable->key);
	registro->timestamp = string_from_format("%llu", aux_memtable->timeStamp);
	registro->value = string_from_format("%s", aux_memtable->value);
	return registro;
}

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

void borrarTabla(char* path){
	DIR* path_buscado = opendir(path);
	struct dirent* archivo = readdir(path_buscado);
	char* path_aux;
	while(archivo){
		path_aux = string_from_format("%s%s", path, archivo->d_name);
		if(access(path_aux, F_OK) != -1){	//Si existe algun archivo y puedo acceder, borralo
//			if(string_ends_with(path_aux, ".bin")){		//si es un .bin, tengo que desenlazar el numero de bloque en el bitmap
//				quitarEnlaceBloques(path_aux);
//			}
			remove(path_aux);
		}
		free(path_aux);
		archivo = readdir(path_buscado);
	}
	closedir(path_buscado);
}

void* dump(int tiempo_dump){
	int tiempo = tiempo_dump/1000;
	t_list* memTableAux = NULL;

	while(1){
		sleep(tiempo);
		if(!list_is_empty(memTable)){
			memTableAux = list_duplicate(memTable);
			list_destroy(memTable); //Hago esto para que se puedan seguir efectuando insert sin complicar las cosas
			memTable = list_create();
			dumpDeTablas(memTableAux);
			log_info(logger, "Dump realizado, numero: %d", cantDumps);
		}else{
			log_info(logger, "No se puede hacer Dump: Memtable vacía");
		}
	}
	return 0;
}

int funcionModulo(int key, int particiones){
	return key % particiones;
}

// ############### Extras ###############

t_config* leer_config() {
	return config_create("LFS.config");
}

t_log* iniciar_logger() {
	return log_create("Lissandra.log", "Lissandra", 1, LOG_LEVEL_TRACE);
}

void agregarAMemTable(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp){
	t_registro *registro = malloc(sizeof(t_registro));
	registro->nombre_tabla = malloc(strlen(nombreTabla)+1);
	registro->value = malloc(strlen(valor)+1);
	strcpy(registro->nombre_tabla, nombreTabla);
	registro->key = key;
	strcpy(registro->value, valor);
	registro->timeStamp = timeStamp;
	list_add(memTable, registro);;
}

uint64_t getTimestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

t_registro* convertirARegistroPuntero(t_registro r){
	t_registro* registro = malloc(sizeof(t_registro));
	*registro = r;
	return registro;
}

struct_describe_respuesta* convertirAPuntero(struct_describe_respuesta describe){
	struct_describe_respuesta* rta = malloc(sizeof(struct_describe_respuesta));
	*rta = describe;
	return rta;
}

struct_select_respuesta convertirARespuestaSelect(datos_a_compactar* registro){
	struct_select_respuesta select_respuesta;
	select_respuesta.estado = ESTADO_SELECT_OK;
	uint64_t timestamp = stringToLongLong(registro->timestamp);
	select_respuesta.timestamp = timestamp;
	select_respuesta.valor = string_from_format("%s", registro->value);
	return select_respuesta;
}

struct_describe_respuesta convertirARespuestaDescribe(char* consistencia, char* particiones, char* compactationTime){
	struct_describe_respuesta describe_respuesta;
	describe_respuesta.estado = ESTADO_DESCRIBE_OK;

	if(!strcmp("SC\n", consistencia)){
		describe_respuesta.consistencia = SC;
	}
	if(!strcmp("SHC\n", consistencia)){
		describe_respuesta.consistencia = SHC;
	}
	if(!strcmp("EC\n", consistencia)){
		describe_respuesta.consistencia = EC;
	}

	char* endptrParticiones, *particionesstr = particiones;
	ulong particiones_l = strtoul(particionesstr, &endptrParticiones, 10);
	describe_respuesta.particiones = particiones_l;

	char* endptrCompactationTime, *compactationTimestr = compactationTime;
	ulong compactationTime_l = strtoul(compactationTimestr, &endptrCompactationTime, 10);
	describe_respuesta.tiempo_compactacion = compactationTime_l;
	return describe_respuesta;
}

bool ordenarDeMayorAMenorTimestamp(t_registro* r1, t_registro* r2){
	return r1->timeStamp>r2->timeStamp;
}

bool ordenarDeMayorAMenorTimestampFinal(datos_a_compactar* r1, datos_a_compactar* r2){
	uint64_t ts_1 = stringToLongLong(r1->timestamp);
	uint64_t ts_2 = stringToLongLong(r2->timestamp);
	log_trace(logger, "%llu > llu ?", ts_1, ts_2);
	return ts_1 > ts_2;
}

void agregarRegDeBloquesYTemps(t_list *lista, char* nombreTabla, u_int16_t key){
	char *path = string_from_format("%sTable/%s", puntoMontaje, nombreTabla);
	DIR* path_buscado = opendir(path);
	free(path);
	struct dirent* carpeta = readdir(path_buscado);

	while(carpeta){
		if(esArchivoValido(carpeta->d_name)){
//			log_trace(logger, "## %s ##", carpeta->d_name);
			char* path_bin = string_from_format("%sTable/%s/%s", puntoMontaje, nombreTabla, carpeta->d_name);
			FILE* f = fopen(path_bin, "r");
			free(path_bin);
			int size_bin = obtenerSizeBin(f);
			char* append = NULL;
			if(size_bin != 0){
//				log_trace(logger, "%s tiene %d datos", carpeta->d_name, size_bin);
				int size_ultimo_bloque = size_bin % blockSize;
				char** bloques = obtenerBloquesBin(f);
				int i = 0;
				while(bloques[i]){
					if(esUltimoBloque(bloques,i)){
						cargarUltimoBloqueSELECT(bloques[i], lista, size_ultimo_bloque, append, key);
					}else{
						cargarBloqueAListaSELECT(bloques[i], lista, append, key);
						append = obtenerUltimaLinea(bloques[i]);
					}
					i++;
				}
				liberarArrayString(bloques);
			}else{
//				log_trace(logger, "%s esta vacio", carpeta->d_name);
			}
			fclose(f);

		}
		carpeta = readdir(path_buscado);
	}
//	log_trace(logger,"Antes del closedir");
	closedir(path_buscado);
//	log_trace(logger,"Despues del closedir");
}

void cargarUltimoBloqueSELECT(char* bloque, t_list* lista, int size_lectura, char* append, u_int16_t key){
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
			cargarLineaSELECT(linea, lista, key);
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
				cargarLineaSELECT(linea, lista, key);
				liberarArrayString(linea);
				free(buffer);
				buffer = NULL;
			}else if(size_actual == size_lectura){
//				log_trace(logger, "%d es igual a %d, salgo del while", size_actual, size_lectura);
				char** linea = string_split(buffer, ";");
				cargarLineaSELECT(linea, lista, key);
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

void cargarLineaSELECT(char** linea, t_list* lista, u_int16_t key){
	u_int16_t key_linea = stringToLong(linea[1]);
	if(key_linea == key){
//		log_trace(logger, "El registro con value %s, tiene igual key", linea[2]);
		datos_a_compactar* registro = malloc(sizeof(datos_a_compactar));
		registro->timestamp = string_from_format("%s", linea[0]);
		registro->key = string_from_format("%s", linea[1]);
		char* value_aux = string_from_format("%s", linea[2]);
		registro->value = string_substring(value_aux, 0, strlen(value_aux)-1);
		free(value_aux);
		list_add(lista, registro);
	}
}

void cargarBloqueAListaSELECT(char* bloque, t_list* lista, char* append, u_int16_t key){
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
			cargarLineaSELECT(linea, lista, key);
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
			cargarLineaSELECT(linea, lista, key);
		}
		liberarArrayString(linea);
		free(buffer);
		buffer = NULL;
	}
	if(buffer){free(buffer);}
	buffer = NULL;

	fclose(block_file);
}

void crearDirectiorioDeTabla(char* nombreTabla){
	char* path = string_from_format("%sTable/%s", puntoMontaje, nombreTabla);
	if(mkdir(path, 0777) != 0){}
	free(path);
}

char* obtenerPrimeraLinea(char* bloque){
	char* path = string_from_format("%sBloques/%s.bin", puntoMontaje, bloque);
	FILE* bloque_bin = fopen(path, "r");
	char* buffer = NULL; size_t buffer_size = 0;

	if(getline(&buffer, &buffer_size, bloque_bin)){
		fclose(bloque_bin);
		return buffer;
	}
	fclose(bloque_bin);
	return NULL;
}

t_registro* creadorRegistroPuntero(u_int16_t key, char* nombreTabla, uint64_t timeStamp, char* value){
	log_trace(logger, "Entro a creadorRegistroPuntero()");
	t_registro* retornado = malloc(sizeof(t_registro));
	retornado->key = key;
	log_trace(logger, "Asigno Key");
	if(nombreTabla){
		retornado->nombre_tabla = malloc(strlen(nombreTabla));
		strcpy(retornado->nombre_tabla, nombreTabla);
	}else{
		log_trace(logger, "No existe nombre de tabla");
		retornado->nombre_tabla = NULL;
	}
	retornado->timeStamp = timeStamp;
	log_trace(logger, "Asigno timestamp");
	retornado->value = string_from_format("%s", value);
	log_trace(logger, "Asigno Value");
	return retornado;
}

