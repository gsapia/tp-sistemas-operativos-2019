#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <string.h>
#include <readline/readline.h>
#include "API.h"

char *apiMemoria(char* mensaje){
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
					switch (resultado.estado) {
						case ESTADO_SELECT_OK:
							return strdup(resultado.valor);
						case ESTADO_SELECT_ERROR_TABLA:
							return strdup("ERROR: La tabla solicitada no existe.");
						case ESTADO_SELECT_ERROR_KEY:
							return strdup("ERROR: Esa tabla no contiene ningun registro con la clave solicitada.");
						case ESTADO_SELECT_MEMORIA_FULL:
							return strdup("ERROR: La memoria esta FULL y no puede procesar la solicitud.");
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
			//INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
			//INSERT TABLA1 3 "Mi nombre es Lissandra"

			free(comando[0]);
			if (cantArgumentos >= 3) {
				char** argumentos = string_n_split(mensaje, 4, " ");
				char** ultimoArgumento = string_split(argumentos[3], "\"");
				free(argumentos[0]);
				free(argumentos[1]);
				free(argumentos[2]);
				free(argumentos[3]);
				free(argumentos);

				if(!ultimoArgumento[1]){ // Si hay un solo argumento sigo, sino es que hay argumentos de mas...
					char* nombreTabla = comando[1];
					char* keystr = comando[2];
					char* endptr;
					ulong key = strtoul(keystr, &endptr, 10);
					char* valor = ultimoArgumento[0];
					if (*endptr == '\0' && key < 65536) {
						enum estados_insert resultado = insert(nombreTabla, key, valor);
						while(cantArgumentos){
							free(comando[cantArgumentos]);
							cantArgumentos--;
						}
						free(valor);
						free(ultimoArgumento);
						free(comando);
						switch (resultado) {
							case ESTADO_INSERT_OK:
								return strdup("Valor insertado");
							case ESTADO_INSERT_ERROR_TABLA:
								return strdup("ERROR: Esa tabla no existe.");
							case ESTADO_INSERT_MEMORIA_FULL:
								return strdup("ERROR: La memoria esta FULL y no puede procesar la solicitud.");
							default:
								return strdup("ERROR: Ocurrio un error desconocido.");
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
			return string_from_format("Sintaxis invalida. Uso: INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”");
		}
		else if(!strcmp(comando[0],"CREATE")){
			//CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
			//CREATE TABLA1 SC 4 60000

			free(comando[0]);
			if(cantArgumentos == 4){
				char* nombreTabla = comando[1];
				char* tipoConsistenciaStr = comando[2];

				enum consistencias tipoConsistencia;

				if(!strcmp(tipoConsistenciaStr, "SC"))
					tipoConsistencia = SC;
				else if(!strcmp(tipoConsistenciaStr, "SHC"))
					tipoConsistencia = SHC;
				else if(!strcmp(tipoConsistenciaStr, "EC"))
					tipoConsistencia = EC;
				else
					return string_from_format("Tipo de consistencia invalido.");

				char* cantidadParticionesstr = comando[3];
				char* compactionTimestr = comando[4];
				char* endptr = 0;
				ulong cantidadParticiones = strtoul(cantidadParticionesstr, &endptr, 10);
				ulong compactionTime;
				if(*endptr == '\0')
					compactionTime = strtoul(compactionTimestr, &endptr, 10);
				if(*endptr == '\0'){
					enum estados_create resultado = create(nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
					free(nombreTabla);
					free(tipoConsistenciaStr);
					free(cantidadParticionesstr);
					free(compactionTimestr);
					free(comando);

					switch (resultado) {
						case ESTADO_CREATE_OK:
							return strdup("Tabla creada");
						case ESTADO_CREATE_ERROR_TABLAEXISTENTE:
							return strdup("ERROR: Esa tabla ya existe.");
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

			free(comando[0]);
			if(cantArgumentos == 1){ // Viene con nombre de tabla
				char* nombreTabla = comando[1];
				struct_describe_respuesta resultado = describe(nombreTabla);
				free(nombreTabla);
				free(comando);
				switch (resultado.estado) {
					case ESTADO_DESCRIBE_OK:
						return string_from_format("Consistencia: %s, Particiones: %d, Tiempo de Compactacion: %ld", consistenciaAString(resultado.consistencia), resultado.particiones, resultado.tiempo_compactacion);
					case ESTADO_DESCRIBE_ERROR_TABLA:
						return strdup("ERROR: Esa tabla no existe.");
					default:
						return strdup("ERROR: Ocurrio un error desconocido.");
				}
			}
			if(cantArgumentos == 0){ // Estamos ante un DESCRIBE global
				free(comando);
				struct_describe_global_respuesta resultado = describe_global();

				switch (resultado.estado) {
					case ESTADO_DESCRIBE_OK:
						{
							char* respuesta = string_new();
							void iterador_describes(char* nombre_tabla, struct_describe_respuesta* describe){
								string_append_with_format(&respuesta, "Tabla: %s Consistencia: %s, Particiones: %d, Tiempo de Compactacion: %ld\n", nombre_tabla, consistenciaAString(describe->consistencia), describe->particiones, describe->tiempo_compactacion);
							}
							dictionary_iterator(resultado.describes, (void(*)(char*,void*))iterador_describes);
							dictionary_destroy_and_destroy_elements(resultado.describes, free);

							return respuesta;
						}
						break;
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

				switch (resultado) {
					case ESTADO_DROP_OK:
						return strdup("Tabla eliminada");
					case ESTADO_DROP_ERROR_TABLA:
						return strdup("ERROR: Esa tabla no existe");
					default:
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
		else if(!strcmp(comando[0],"JOURNAL")){
			//JOURNAL

			free(comando[0]);
			if(cantArgumentos == 0){
				free(comando);
				enum estados_journal resultado = journal();
				switch (resultado) {
					case ESTADO_JOURNAL_OK:
						return strdup("JOURNAL OK");
					default:
						return strdup("ERROR: Ocurrio un error desconocido.");
				}
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: JOURNAL");
		}
		else if(!strcmp(comando[0],"ESTADO")){
			//ESTADO

			free(comando[0]);
			if(cantArgumentos == 0){
				free(comando);
				return estado();
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: JOURNAL");
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

void consola() {
	char *linea;
	char *resultado;
	while (1) {
		linea = readline(">");
		if(linea){
			if (!strcmp(linea, "exit")) {
				free(linea);
				break;
			}

			resultado = apiMemoria(linea);
			free(linea);
			string_trim(&resultado);
			puts(resultado);
			free(resultado);
		}
	}
}
