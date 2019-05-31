#include "LFS.h"
#include "serializacion.h"
void* consola();
char* apiLissandra(char*);
struct_select_respuesta selects(char* nombreTabla, u_int16_t key);
char* insert(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp);
uint16_t create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
char* describe(char* nombreTabla);
char* drop(char* nombreTabla);
bool existeTabla(char* nombreTabla);
FILE* obtenerMetaDataLectura(char* nombreTabla);
int funcionModulo(int key, int particiones);
int obtenerParticiones(FILE* metadata);
char* obtenerValue(char* nombreTabla, u_int16_t key, int particion);
FILE* obtenerBIN(int particion, char* nombreTabla);
void crearDirectiorioDeTabla(char* nombreTabla);
void crearMetadataDeTabla(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
void crearBinDeTabla(char* nombreTabla, int cantParticiones);
void dumpDeTablas(t_list *memTableAux);
void agregarAMemTable(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp);
uint64_t getTimestamp();
void filtrarKeyRegDeMemTable(t_list *listaFiltro, u_int16_t key);
void obtenerRegistrosDeTable(t_list *listaFiltro, u_int16_t key, int particion_busqueda, char* nombreTabla);
struct_select_respuesta regMayorTimestamp(t_list* listaFiltro);
t_registro* convertirARegistroPuntero(t_registro r);
struct_select_respuesta convertirARespuestaSelect(t_registro* mayor);

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
	return 0;
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
					switch (resultado.estado){
						case ESTADO_SELECT_OK:
							return strdup(resultado.valor);
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
						char* resultado = insert(nombreTabla, key, valor, timestamp);
						while(cantArgumentos){
							free(comando[cantArgumentos]);
							cantArgumentos--;
						}
						free(valor);
						free(ultimoArgumento);
						free(comando);
						return resultado;
					}
				}else{ // Si viene CON TIMESTAMP
					char* nombreTabla = comando[1];
					char* keystr = comando[2];
					char* endptr;
					ulong key = strtoul(keystr, &endptr, 10);
					char* valor = ultimoArgumento[0];
					uint64_t timestamp = atof(ultimoArgumento[1]);
					if (*endptr == '\0' && key < 65536) {
						char* resultado = insert(nombreTabla, key, valor, timestamp);
						while(cantArgumentos){
							free(comando[cantArgumentos]);
							cantArgumentos--;
						}
						free(valor);
						free(ultimoArgumento);
						free(comando);
						return resultado;
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
			if(cantArgumentos == 1){
				char* nombreTabla = comando[1];
				char* resultado = describe(nombreTabla);
				free(nombreTabla);
				free(comando);
				return resultado;
			} else if(cantArgumentos == 0){
				char* resultado = describe(NULL);
				return resultado;
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
	t_list *listaFiltro = list_create();

	if(existeTabla(nombreTabla)){
		FILE* metadata = obtenerMetaDataLectura(nombreTabla);
		int particiones = obtenerParticiones(metadata);
		int particion_busqueda = funcionModulo(key, particiones);

//		### Meto a todos los registros con la misma key en en una lista ###
		filtrarKeyRegDeMemTable(listaFiltro, key);
		obtenerRegistrosDeTable(listaFiltro, key, particion_busqueda, nombreTabla);
//		obtenerRegDeArchivosTemp();
		select_respuesta = regMayorTimestamp(listaFiltro);

		fclose(metadata);
		list_destroy_and_destroy_elements(listaFiltro, free);
		log_debug(logger, "SELECT: Recibi Tabla: %s Key: %d", nombreTabla, key);
		return select_respuesta;
	}else{
		select_respuesta.estado = ESTADO_SELECT_ERROR_TABLA;
		list_destroy_and_destroy_elements(listaFiltro, free);
		log_debug(logger, "No existe en el File System la tabla: %s",nombreTabla);
		return select_respuesta;
	}

}

char* insert(char* nombreTabla, u_int16_t key, char* valor, uint64_t timeStamp){
	if(existeTabla(nombreTabla)){
		agregarAMemTable(nombreTabla, key, valor, timeStamp);
		log_debug(logger, "INSERT: Tabla: %s, Key: %d, Valor: %s, Timestamp: %f", nombreTabla, key, valor, timeStamp);
		return string_from_format("Se realizo el INSERT");
	}else{
		log_debug(logger, "No existe en el File System la tabla: %s",nombreTabla);
		return string_from_format("No existe en el File System la tabla: %s",nombreTabla);
	}
}

uint16_t create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
//	CREATE TABLA1 SC 4 60000
	uint16_t estado;
	if(!existeTabla(nombreTabla)){
		crearDirectiorioDeTabla(nombreTabla);
		crearMetadataDeTabla(nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
		crearBinDeTabla(nombreTabla, cantidadParticiones);
		log_debug(logger, "CREATE: Recibi Tabla: %s TipoDeConsistencia: %s CantidadDeParticines: %d TiempoDeCompactacion: %d", nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
		estado = ESTADO_CREATE_OK;
	}else{
		log_info(logger, "La tabla %s ya existe en el FS", nombreTabla);
		estado = ESTADO_CREATE_ERROR_TABLAEXISTENTE;
	}
	return estado;
}

char* describe(char* nombreTabla){
	if(nombreTabla){
		log_debug(logger, "DESCRIBE: Recibi Tabla: %s", nombreTabla);
	}else{
		log_info(logger, "NO me pasaron una tabla con DESCRIBE");
	}

	return string_from_format("Elegiste DESCRIBE");
}

char* drop(char* nombreTabla){
	t_registro *imprimir;
	for(int i=0;i<cont;i++){
			imprimir = list_get(memTable,i);
			printf("Nombre de Tabla: %s \nKey: %u \nValue: %s \n", imprimir->nombre_tabla, imprimir->key, imprimir->value);
		}
		cont = 0;
	log_debug(logger, "DROP: Recibi Tabla: %s", nombreTabla);
	return string_from_format("Elegiste DROP");
}

//Descarga toda la informacion de la memtable, de todas las tablas, y copia dichos datos en los ditintos archivos temporales (uno por tabla). Luego se limpia la memtable.
void* dump(int tiempo_dump){
	int tiempo = tiempo_dump/1000;
	t_list* memTableAux = list_create();

	while(1){
		sleep(tiempo);
		if(!list_is_empty(memTable)){
			memTableAux = list_duplicate(memTable);
			list_destroy_and_destroy_elements(memTable, free); //Hago esto para que se puedan seguir efectuando insert sin complicar las cosas
			dumpDeTablas(memTableAux); //
			log_info(logger, "Dump realizado, numero: %d", cantDumps+1);
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

char* obtenerValue(char* nombreTabla, u_int16_t key, int particion){
	char* value = "Valor Default";
	int contador = 0;
	void *listaDeReg = list_create();
	t_registroBusqueda *reg = malloc(sizeof(t_registroBusqueda));
	FILE* bin = obtenerBIN(particion, nombreTabla);

	reg->key = 10;
	reg->timeStamp = 400;
	reg->value = "Value 1";
	fwrite(&reg, sizeof(t_registroBusqueda), 1, bin);
	reg->key = 10;
	reg->timeStamp = 100;
	reg->value = "Value 2";
	fwrite(&reg, sizeof(t_registroBusqueda), 1, bin);
	reg->key = 10;
	reg->timeStamp = 200;
	reg->value = "Value 3";
	fwrite(&reg, sizeof(t_registroBusqueda), 1, bin);

	while(!feof(bin)){
		t_registroBusqueda *reg2;
		fread(&reg2, sizeof(t_registroBusqueda), 1, bin);
		if(reg->key == key){
			list_add(listaDeReg, reg2);
			contador++;
		}
	}
	printf("Contador = %d \n", contador);
	t_registroBusqueda *aux1;
	t_registroBusqueda *aux2;
	if(contador > 0){
		if(contador == 1){
			aux1 = list_get(listaDeReg,0);
			value = aux1->value;
		}else if(contador == 2){
			printf("Contador == 2");
			aux1 = list_get(listaDeReg,0);
			aux2 = list_get(listaDeReg,1);
			if(aux1->timeStamp > aux2->timeStamp){
				value = aux1->value;
			}else{
				value = aux2->value;
			}
		}else{
			aux1 = list_get(listaDeReg,0);
			aux2 = list_get(listaDeReg,1);
			for(int i=2;i<contador;i++){
				if(aux1->timeStamp > aux2->timeStamp){
					aux2 = list_get(listaDeReg,i);
				}else{
					aux1 = list_get(listaDeReg,i);
				}
			}
			if(aux1->timeStamp > aux2->timeStamp){
				value = aux1->value;
			}else{
				value = aux2->value;
			}

		}
	}else{
		printf("No hay registros con esa Key");
	}

	free(reg);
	fclose(bin);
	return value;
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

// Devuelve una lista con todos los registros de la Memtable que tengan dicha key
void filtrarKeyRegDeMemTable(t_list *listaFiltro, u_int16_t key){
	int cant = list_size(memTable);
	t_registro *aux = malloc(sizeof(t_registro));
	for(int i=0;i<cant;i++){
		aux = list_get(memTable, i);
		if(aux->key == key){
			list_add(listaFiltro, aux);
		}
	}
}

// Dado un archivo bin, busca en el archivo los registros que tengan la key igual a la pasada
//  por parametro y los agrega a listaFiltro
void obtenerRegistrosDeTable(t_list *listaFiltro, u_int16_t key, int particion_busqueda, char* nombreTabla){
	FILE* bin = obtenerBIN(particion_busqueda, nombreTabla);
	t_registro reg;
	t_registro* aux;
	while(!feof(bin)){
		fread(&reg, sizeof(reg),1,bin);
		if(reg.key == key){
			aux = convertirARegistroPuntero(reg);
			list_add(listaFiltro, aux);
		}
	}
}

// Busca dentro de una lista, el registro con mayor timestamp y lo devuelve en modo de select
struct_select_respuesta regMayorTimestamp(t_list* listaFiltro){
	struct_select_respuesta regSelect;
	t_registro *mayor = malloc(sizeof(t_registro));
	t_registro *aux = malloc(sizeof(t_registro));
	mayor->timeStamp = -1;
	mayor->nombre_tabla =malloc(50);
	mayor->value = malloc(50);

	if(!list_is_empty(listaFiltro)){
		while(!list_is_empty(listaFiltro)){
			aux = list_remove(listaFiltro, 0);
			if(aux->timeStamp > mayor->timeStamp){
				mayor->key = aux->key;

				strcpy(mayor->nombre_tabla,aux->nombre_tabla);
				mayor->timeStamp = aux->timeStamp;
				strcpy(mayor->value,aux->value);
			}
		}
		regSelect = convertirARespuestaSelect(mayor);
		free(mayor);
		free(aux);
		return regSelect;
	}else{
		regSelect.estado = ESTADO_SELECT_ERROR_KEY;
		return regSelect;
	}
}

//Convierte un t_registro a un t_registro*
t_registro* convertirARegistroPuntero(t_registro r){
	t_registro* registro = malloc(sizeof(t_registro));
	*registro = r;
	return registro;
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
