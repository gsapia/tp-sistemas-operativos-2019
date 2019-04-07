#include "Memoria.h"

void* consola();
char* apiMemoria(char*);

char* selects(char* nombreTabla, u_int16_t key);
// ^ La funcion select() ya existe, hay que buscar otro nombre mas creativo.... ^
char* insert(char* nombreTabla, u_int16_t key, char* valor);
char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
// ^ Cantidad de particiones puede ser mas grande que u_int??? Lo mismo para compactionTime ^
// ^ Ademas mas abajo asumo que son mayores a 0, quizas esta mal? ^
// ^^ tipoConsistencia podria ser un enum?? ^^
char* describe(char* nombreTabla);
char* drop(char* nombreTabla);
char* journal();

t_log* logger;


int main(void) {
	logger = log_create("Memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);
	log_info(logger, "Hola Soy Memoria");
	pthread_t hiloConsola;
	int hilo = pthread_create(&hiloConsola, NULL, consola, NULL);
	if(hilo){
		log_error(logger, "Hilo consola: Error - pthread_create() return code: %d\n", hilo);
		exit(EXIT_FAILURE);
	}

	pthread_join(hiloConsola, NULL);

	// 1. Conectarse al proceso File System y realizar handshake necesario para obtener los datos requeridos. Esto incluye el tamaño máximo del Value configurado para la administración de las páginas.

	// 2. Inicializar la memoria principal (que se explican en los siguientes apartados).

	// 3. Iniciar el proceso de Gossiping (explicado en profundidad en el Anexo III) que consiste en la comunicación de cada proceso memoria con otros procesos memorias, o seeds, para intercambiar y descubrir otros procesos memorias que se encuentren dentro del pool (conjunto de memorias).

	//En caso que no pueda realizar alguna de las dos primeras operaciones, se deberá abortar el proceso memoria informando cuál fue el problema.

	return EXIT_SUCCESS;
}

void* consola(){
	char *linea;
	char *resultado;
	while(1) {
		linea = readline(">");

		resultado = apiMemoria(linea);
		free(linea);
		puts(resultado);
		free(resultado);

	}
}

char *apiMemoria(char* mensaje){
	char* comando = strtok(mensaje, " ");
	if(comando){
		if(!strcmp(comando,SELECT)){
			//SELECT [NOMBRE_TABLA] [KEY]
			//SELECT TABLA1 3

			char* nombreTabla = strtok(NULL, " ");
			char* keystr = strtok(NULL, " ");
			char* endptr;
			ulong key;
			if(keystr){
				key = strtoul(keystr, &endptr, 10);
			}
			if(!nombreTabla || !endptr || *endptr != '\0'|| (key > 65536 || key < 0) || strtok(NULL, " ")){
				return strdup("Sintaxis invalida. Uso: SELECT [NOMBRE_TABLA] [KEY]");
			}
			return selects(nombreTabla, key);
		}
		else if(!strcmp(comando,INSERT)){
			//INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
			//INSERT TABLA1 3 “Mi nombre es Lissandra”

			char* nombreTabla = strtok(NULL, " ");
			char* keystr = strtok(NULL, " ");
			char* endptr= 0;
			ulong key;
			if(keystr){
				key = strtoul(keystr, &endptr, 10);
			}
			char* valor = strtok(NULL, " ");
			if(!nombreTabla || !endptr || *endptr != '\0'|| key > 65536 || !valor || strtok(NULL, " ")){
				return strdup("Sintaxis invalida. Uso: INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”");
			}
			return insert(nombreTabla, key, valor);
		}
		else if(!strcmp(comando,CREATE)){
			//CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
			//CREATE TABLA1 SC 4 60000

			char* nombreTabla = strtok(NULL, " ");
			char* tipoConsistencia = strtok(NULL, " ");

			char* cantidadParticionesstr = strtok(NULL, " ");
			char* compactionTimestr = strtok(NULL, " ");
			char* endptr = 0;
			ulong cantidadParticiones;
			ulong compactionTime;
			if(cantidadParticionesstr && compactionTimestr){
				cantidadParticiones = strtoul(cantidadParticionesstr, &endptr, 10);
				if(*endptr == '\0')
					compactionTime = strtoul(compactionTimestr, &endptr, 10);
			}
			if(!nombreTabla || !tipoConsistencia || !endptr || *endptr != '\0' || !cantidadParticiones || !compactionTime || strtok(NULL, " ")){
				// Faltaria revisar el tipo de consistencia ^
				return strdup("Sintaxis invalida. Uso: CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]");
			}
			return create(nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
		}
		else if(!strcmp(comando,DESCRIBE)){
			//DESCRIBE [NOMBRE_TABLA]
			//DESCRIBE TABLA1

			char* nombreTabla = strtok(NULL, " ");
			if(!nombreTabla || strtok(NULL, " ")){
				return strdup("Sintaxis invalida. Uso: DESCRIBE [NOMBRE_TABLA]");
			}
			return describe(nombreTabla);
		}
		else if(!strcmp(comando,DROP)){
			//DROP [NOMBRE_TABLA]
			//DROP TABLA1

			char* nombreTabla = strtok(NULL, " ");
			if(!nombreTabla || strtok(NULL, " ")){
				return strdup("Sintaxis invalida. Uso: DROP [NOMBRE_TABLA]");
			}
			return drop(nombreTabla);
		}
		else if(comando && !strcmp(comando,JOURNAL)){
			//JOURNAL

			if(strtok(NULL, " ")){
				return strdup("Sintaxis invalida. Uso: JOURNAL");
			}
			return journal();
		}
	}
	return strdup("Comando invalido");
}

// Abajo uso strdup() porque ahora solo uso string literals (no es memoria dinamica), y por ende free() rompe.
// Habria que ver despues si eso es necesario, o tratando los resultados como const char* es suficiente
char* selects(char* nombreTabla, u_int16_t key){
	log_debug(logger, "SELECT: Recibi Tabla:%s Key:%d", nombreTabla, key);
	return strdup("Elegiste SELECT");
}
char* insert(char* nombreTabla, u_int16_t key, char* valor){
	log_debug(logger, "INSERT: Recibi Tabla:%s Key:%d Valor:%s", nombreTabla, key, valor);
	return strdup("Elegiste INSERT");
}
char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	log_debug(logger, "CREATE: Recibi Tabla:%s TipoDeConsistencia:%s CantidadDeParticines:%d TiempoDeCompactacion:%d", nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
	return strdup("Elegiste CREATE");
}
char* describe(char* nombreTabla){
	log_debug(logger, "DESCRIBE: Recibi Tabla:%s", nombreTabla);
	return strdup("Elegiste DESCRIBE");
}
char* drop(char* nombreTabla){
	log_debug(logger, "DROP: Recibi Tabla:%s", nombreTabla);
	return strdup("Elegiste DROP");
}
char* journal(){
	return strdup("Elegiste JOURNAL");
}
