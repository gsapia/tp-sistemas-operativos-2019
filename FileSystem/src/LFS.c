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
					char** valorS = string_split(resultado.valor, "\n");
					free(resultado.valor);
					char* valor = valorS[0];
					free(valorS);
					switch (resultado.estado){
						case ESTADO_SELECT_OK:
							return valor;
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

				switch (resultado.estado){
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
				char* resultado = drop(nombreTabla);
				free(nombreTabla);
				free(comando);
				return resultado;
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
	t_registro *aux;
	t_list *listaFiltro = NULL;

	if(existeTabla(nombreTabla)){
		bool registro_IgualKey(t_registro *registro){return registro->key == key && !strcmp(registro->nombre_tabla,nombreTabla);}

		listaFiltro = list_filter(memTable, (_Bool (*)(void*))registro_IgualKey);
		agregarRegDeBloquesYTemps(listaFiltro, nombreTabla, key);

		if(!list_is_empty(listaFiltro)){
			list_sort(listaFiltro, (_Bool (*)(void*,void*))ordenarDeMayorAMenorTimestamp);
			aux = list_get(listaFiltro, 0);
			select_respuesta = convertirARespuestaSelect(aux);
		}else{
			select_respuesta.estado = ESTADO_SELECT_ERROR_KEY;
		}

		list_destroy(listaFiltro);
		log_debug(logger, "SELECT: Recibi Tabla: %s Key: %d", nombreTabla, key);
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

		//Creo el hilo
		pthread_attr_t attr;
		pthread_t hiloCompactacion;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		hiloC->attrHilo = &attr;
		list_add(hilosCompactacion, hiloC);
		if(pthread_create(&hiloCompactacion, &attr, compactacion, args)){
			log_error(logger, "Hilo compactacion: Error - pthread_create()");
			exit(EXIT_FAILURE);
		}

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

char* drop(char* nombreTabla){
	t_hiloCompactacion *hiloCompactacion;
	if(existeTabla(nombreTabla)){	//Tengo certeza que existe la tabla, entonces va a estar en la lista
		char* path = string_from_format("%sTable/%s/", puntoMontaje, nombreTabla);

		bool registro_IgualNombreTabla(t_hiloCompactacion *registro){return !strcmp(registro->nombreTabla,nombreTabla);}

		borrarTabla(path);
		log_trace(logger, "Antes de borrar el path");
		rmdir(path);
		free(path);
		log_trace(logger, "Borro path");

		log_trace(logger, "Empiezo a borrar el hilo");
		hiloCompactacion = list_remove_by_condition(hilosCompactacion, (_Bool (*)(void*))registro_IgualNombreTabla);	//Dame al registro que tenga el mismo nombre de tabla
		pthread_attr_destroy(hiloCompactacion->attrHilo);
		free(hiloCompactacion->nombreTabla);
		free(hiloCompactacion);
		log_trace(logger, "Elimino el hilo");

		log_debug(logger, "DROP: Recibi Tabla: %s", nombreTabla);
	}else{
		log_debug(logger, "DROP: Recibi Tabla: %s", nombreTabla);
		log_error(logger, "No existe la tabla: %s", nombreTabla);
	}

	return string_from_format("Elegiste DROP");
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
/*
void quitarEnlaceBloques(char* bin_string){
	FILE* bin = fopen(bin_string, "r");
	char* bloques = obtenerBloquesDetabla(bin);		// 0 // 4,2,6,1
	if(strlen(bloques)==1){		//Si hay un solo bloque, no me molesto en separarlo y tener que usar un array de strings
		int bloque = stringToLong(bloques);
		bitarray_clean_bit(bitarray, bloque);
	}else{
		char** bloquesS = string_split(bloques, ",");	// 0 4 6 1
		int i = 0;
		while(bloquesS[i]){
			int bloque = stringToLong(bloquesS[i]);
			bitarray_clean_bit(bitarray, bloque);
			free(bloquesS[i]);
			i++;
		}
		free(bloquesS);
	}
	log_trace(logger, "Termino de quitar enlace de bloques");
	free(bloques);
}
*/
//Descarga toda la informacion de la memtable, de todas las tablas, y copia dichos datos en los ditintos archivos temporales (uno por tabla). Luego se limpia la memtable.
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

//Distribuye las disitntas Key dentro de dicha tabla. Se dividirá la key por la cantidad de particiones y el resto de la operación será la partición a utilizar
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

// Dado un archivo bin, busca en el archivo los registros que tengan la key igual a la pasada
//  por parametro y los agrega a lista.
void obtenerRegistrosDeTable(t_list *lista, u_int16_t key, int particion_buscada, char* nombreTabla){
	FILE* bin = obtenerBIN(particion_buscada, nombreTabla);
	size_t buffer_size = 80;
	char* buffer = malloc(buffer_size * sizeof(char));
	t_registro* aux = malloc(sizeof(t_registro));
	while(getline(&buffer, &buffer_size, bin) != -1){ // [TIMESTAMP;KEY;VALUE]
		char** linea= string_split(buffer, ";");
		char* keystr = malloc(sizeof(linea[1]));
		strcpy(keystr, linea[1]);
		char* endptr;
		ulong key_buffer = strtoul(keystr, &endptr, 10);

		if(key_buffer==key){
			char* timestampstr = malloc(sizeof(linea[0]));
			strcpy(timestampstr, linea[0]);
			aux = creadorRegistroPuntero(key, nombreTabla, timestampstr, linea[2]);
			list_add(lista,aux);
			free(timestampstr);
		}
		free(keystr);
		free(linea);
	}
	free(buffer);
	fclose(bin);
}

//Convierte un t_registro a un t_registro*
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

//Convierte a un t_registro* a un struct_select_respuesta
struct_select_respuesta convertirARespuestaSelect(t_registro* registro){
	struct_select_respuesta select_respuesta;
	select_respuesta.estado = ESTADO_SELECT_OK;
	select_respuesta.timestamp = registro->timeStamp;
	select_respuesta.valor = malloc(strlen(registro->value)+1);
	strcpy(select_respuesta.valor,registro->value);
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

//Obtiene la particion sobre la cual debe actuar
int obtenerParticion(char* nombreTabla, u_int16_t key){
	FILE* metadata = obtenerMetaDataLectura(nombreTabla);
	int particiones = obtenerParticiones(metadata);
	int particion_busqueda = funcionModulo(key, particiones);
	fclose(metadata);
	return particion_busqueda;
}

bool ordenarDeMayorAMenorTimestamp(t_registro* r1, t_registro* r2){
	return r1->timeStamp>r2->timeStamp;
}

void agregarRegDeBloquesYTemps(t_list *lista, char* nombreTabla, u_int16_t key){
	agregarRegistrosTempYTempc(nombreTabla, lista, key);	//Agrego de los tmp y tmpc de "/Table/An.tmp" y "/Table/An.tmpc", el registro con el mayor timestamp a la lista.
	agregarRegistroBloques(nombreTabla, lista, key); 	//Agrego del /Bloques/n.bin, el registro con el mayor timestamp a la lista
}

void agregarRegistroBloques(char* nombreTabla, t_list *lista, u_int16_t key){
	int particion = obtenerParticion(nombreTabla, key);
	char* path_bin = string_from_format("%sTable/%s/%d.bin", puntoMontaje, nombreTabla, particion);
	FILE* binTabla = fopen(path_bin, "r+");
	char* lineaBin = NULL; size_t lineaBin_size = 0;

	if(getline(&lineaBin, &lineaBin_size, binTabla) != -1){		//SIZE=250
		free(lineaBin);
		lineaBin = NULL;
		if(getline(&lineaBin, &lineaBin_size, binTabla) != -1){	//char BLOCKS=[40,21,82,3]
			char** bloques = string_split(lineaBin, "=");	//char [BLOCKS,[40,21,82,3]]
			char** blocks = obtenerBloques(bloques[1]);		//char [40,21,82,3]

			if(existeKeySELECT(key, blocks, lista)){	//Como en los bloques solo va a existir una sola entrada de la key y va a ser la mas actualizada por Compactacion, solo tengo que econtrar que exista
				//log_trace(logger, "Existe key %d", key);
			}else{
				//log_trace(logger, "No existe key %d", key);
			}

			int j=0;
			while(bloques[j]){
				free(bloques[j]);
				j++;
			}
			free(bloques);
		}
	}
	fclose(binTabla);
}

bool existeKeySELECT(u_int16_t key, char** bloques, t_list* lista){
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
					fclose(bloque_bin);
					uint64_t timestamp = stringToLong(line[0]);
					t_registro* registro = creadorRegistroPuntero(key, NULL, timestamp, line[2]);
					list_add(lista,registro);
					free(line[0]);free(line[1]);free(line[2]);free(line);
					return true;
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
						uint64_t timestamp = stringToLong(line[0]);
						log_trace(logger, "Asigno TimeStamp");
						t_registro* registro = creadorRegistroPuntero(key, NULL, timestamp, line[2]);
						log_trace(logger, "Creo el registro");
						list_add(lista,registro);
						log_trace(logger,  "Lo añado");
						free(line[0]);free(line[1]);free(line[2]);free(line);
						log_trace(logger,"Free line");
						fclose(bloque_bin);
						log_trace(logger,"Cierro el archivo bloque");
						return true;
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
		fclose(bloque_bin);
		i++;
	}
	return false;
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

void agregarRegistrosTempYTempc(char* nombreTabla, t_list* lista, u_int16_t key){
	int particion_busqueda = obtenerParticion(nombreTabla, key);
	char* particion = intToString(particion_busqueda);
	char* path = string_from_format("%sTable/%s/%s.bin", puntoMontaje, nombreTabla, particion);
	free(particion);
	free(path);
	FILE* f = fopen(path, "r");

//	log_trace(logger,"Antes del .tmp");
	for(int i=0;i<cantDumps;i++){	// agrego de los .tmp, el registro con el mayor timestamp de cada uno a la lista
		path = string_from_format("%sTable/%s/A%d.tmp", puntoMontaje, nombreTabla, i);
		if(access(path, F_OK) != -1){
			f = fopen(path, "r");
		}
		free(path);
	}

//	log_trace(logger,"Antes del .tmpc");
	int i = 0;
	path = string_from_format("%sTable/%s/A%s.tmpc", puntoMontaje, nombreTabla, i);
	while(access(path, F_OK) != -1){// agrego de los .tmpc, el registro con el mayor timestamp de cada uno a la lista
		free(path);
		f = fopen(path, "r");

		i++;
		path = string_from_format("%s/A%d.tmpc", path, i);
	}
	free(path);
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

