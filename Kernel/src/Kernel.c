#include "Kernel.h"


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

	//Funciones propias del Kernel

	char* add();
	char* run();
	void metrics();


	t_log* logger;


int main(void) {
	puts("Soy Kernel");

	/*int socketPedido;
		struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = INADDR_ANY;
		direccionServidor.sin_port = htons(6060); //Puerto que tiene que escuchar


	    socketPedido = socket (AF_INET ,SOCK_STREAM ,0); //Pedimos un socket enviandole parametros que especifica que utilizamos protocolo TCP/ IP

	    if (bind(socketPedido, (void*) &direccionServidor ,sizeof(direccionServidor)) != 0) //Colomos un bind para que no rompa el codigo.
	    {                                                                                   //Si falla me devuelve un codigo != de cero.
	    	perror("Fallo el bind!");
	    	return 1;

	    }//End if

	    printf("Estoy escuchando\n");
	    listen(socketPedido, SOMAXCONN); // Le pasamos como parametros el socket y un maximo de conexiones, esas conexiones iran a una cola
	                                    // las cuales iremos aceptando. (SOMAXCONN es el numero maximo de conexiones) */


  //****************************** falta terminar socket*****************************


	return EXIT_SUCCESS;

} // end main

char *apiMemoria(char* mensaje){
	char** comando = string_split(mensaje, " ");
	if(comando[0]){
		u_int16_t cantArgumentos = 0;
		while(comando[cantArgumentos+1]){
			cantArgumentos++;
		}

		if(!strcmp(comando[0],SELECT)){
			//SELECT [NOMBRE_TABLA] [KEY]
			//SELECT TABLA1 3

			free(comando[0]);
			if(cantArgumentos == 2){
				char* nombreTabla = comando[1];
				char* keystr = comando[2];
				char* endptr;
				ulong key = strtoul(keystr, &endptr, 10);
				if(*endptr == '\0'&& key < 65536){
					char* resultado = selects(nombreTabla, key);
					free(nombreTabla);
					free(keystr);
					free(comando);
					return resultado;
				}
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: SELECT [NOMBRE_TABLA] [KEY]");
		}
		else if(!strcmp(comando[0],INSERT)){
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
						char* resultado = insert(nombreTabla, key, valor);
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
			return string_from_format("Sintaxis invalida. Uso: INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”");
		}
		else if(!strcmp(comando[0],CREATE)){
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
					char* resultado = create(nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
					free(nombreTabla);
					free(tipoConsistencia);
					free(cantidadParticionesstr);
					free(compactionTimestr);
					free(comando);
					return resultado;
				}
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]");
		}
		else if(!strcmp(comando[0],DESCRIBE)){
			//DESCRIBE [NOMBRE_TABLA]
			//DESCRIBE TABLA1

			free(comando[0]);
			if(cantArgumentos == 1){
				char* nombreTabla = comando[1];
				char* resultado = describe(nombreTabla);
				free(nombreTabla);
				free(comando);
				return resultado;
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			return string_from_format("Sintaxis invalida. Uso: DESCRIBE [NOMBRE_TABLA]");
		}
		else if(!strcmp(comando[0],DROP)){
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
		else if(!strcmp(comando[0],JOURNAL)){
			//JOURNAL

			free(comando[0]);
			if(cantArgumentos == 0){
				free(comando);
				return journal();
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


char* selects(char* nombreTabla, u_int16_t key){
	log_debug(logger, "SELECT: Recibi Tabla:%s Key:%d", nombreTabla, key);
	return string_duplicate("Elegiste SELECT");
}
char* insert(char* nombreTabla, u_int16_t key, char* valor){
	log_debug(logger, "INSERT: Recibi Tabla:%s Key:%d Valor:%s", nombreTabla, key, valor);
	return string_from_format("Elegiste INSERT");
}
char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	log_debug(logger, "CREATE: Recibi Tabla:%s TipoDeConsistencia:%s CantidadDeParticines:%d TiempoDeCompactacion:%d", nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
	return string_from_format("Elegiste CREATE");
}
char* describe(char* nombreTabla){
	log_debug(logger, "DESCRIBE: Recibi Tabla:%s", nombreTabla);
	return string_from_format("Elegiste DESCRIBE");
}
char* drop(char* nombreTabla){
	log_debug(logger, "DROP: Recibi Tabla:%s", nombreTabla);
	return string_from_format("Elegiste DROP");
}
char* journal(){
	return string_from_format("Elegiste JOURNAL");
}


