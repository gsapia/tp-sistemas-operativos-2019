#include "LFS.h"
void* consola();
char* apiLissandra(char*);
char* selects(char* nombreTabla, u_int16_t key);
char* insert(char* nombreTabla, u_int16_t key, char* valor);
char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
char* describe(char* nombreTabla);
char* drop(char* nombreTabla);
bool existeTabla(char* nombreTabla);
FILE* obtenerMetaDataLectura(char* nombreTabla);
int funcionModulo(int key, int particiones);
int obtenerParticiones(FILE* metadata);
char* obtenerValue(char* nombreTabla, u_int16_t key, int particion);
FILE* obtenerBIN(int particion, char* nombreTabla);

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
}

char *apiLissandra(char* mensaje){
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
	char* valueReturn;
	if(existeTabla(nombreTabla)){
		FILE* metadata = obtenerMetaDataLectura(nombreTabla);
		int particiones = obtenerParticiones(metadata);
		printf("Obtuve la cantidad de particiones: %d \n", particiones);
		int particion = funcionModulo(key, particiones);
		printf("La Particion a la que hay que ir a buscar es: %d \n", particion);
		valueReturn = obtenerValue(nombreTabla, key, particion);

		fclose(metadata);
		log_debug(logger, "SELECT: Recibi Tabla: %s Key: %d", nombreTabla, key);
	}else{
		log_debug(logger, "No existe en el File System la tabla: ",nombreTabla);
	}
	return string_from_format("El value es: %s",valueReturn);
}

char* insert(char* nombreTabla, u_int16_t key, char* valor){

	if(existeTabla(nombreTabla)){
		FILE* metadata = obtenerMetaDataLectura(nombreTabla);
		t_registro *registro = malloc(sizeof(t_registro));
		registro->nombre_tabla = malloc(31);
		registro->value = malloc(51);
		strcpy(registro->nombre_tabla, nombreTabla);
		registro->key = key;
		strcpy(registro->value, valor);
//		registro.timestamp;

		list_add(memTable, registro);
		cont++;

		fclose(metadata);
		log_debug(logger, "INSERT: Recibi Tabla: %s Key: %d Valor: %s", nombreTabla, key, valor);
	}else{
		log_debug(logger, "No existe en el File System la tabla: ",nombreTabla);
	}
	return string_from_format("Elegiste INSERT");

}

char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	log_debug(logger, "CREATE: Recibi Tabla: %s TipoDeConsistencia: %s CantidadDeParticines: %d TiempoDeCompactacion: %d", nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
	return string_from_format("Elegiste CREATE");
}

char* describe(char* nombreTabla){
	log_debug(logger, "DESCRIBE: Recibi Tabla: %s", nombreTabla);
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
void dump(){
	printf("Hago Dump\n");
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
/*	reg->key = 10;
	reg->timeStamp = 100;
	reg->value = "Value 2";
	fwrite(&reg, sizeof(t_registroBusqueda), 1, bin);
	reg->key = 10;
	reg->timeStamp = 200;
	reg->value = "Value 3";
	fwrite(&reg, sizeof(t_registroBusqueda), 1, bin);
*/
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


	fclose(bin);
	return value;
}
// ############### SOCKET SERVIDOR ###############

void* servidor(uint16_t puerto_escucha){
	log_trace(logger, "Iniciando servidor");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(puerto_escucha);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor,&direccionServidor,sizeof(direccionServidor))){
		log_error(logger, "Fallo el servidor");
		exit(EXIT_FAILURE); ///
	}

	listen(servidor,SOMAXCONN);
	log_trace(logger, "Escuchando");

	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion = sizeof(direccionCliente);
	int cliente = accept(servidor, &direccionCliente, &tamanoDireccion);
	log_trace(logger, "Recibi una conexion en %d", cliente);


	send(cliente, "Hola soy FS", sizeof("Hola soy FS"), 0);
	while(1){
		char* buffer = malloc(sizeof("Hola soy Memoria"));

		int bytesRecibidos = recv(cliente, buffer, sizeof("Hola soy Memoria"), 0);
		if(bytesRecibidos < 0){
			log_error(logger, "El cliente se desconecto");
			exit(EXIT_FAILURE);
		}
		log_trace(logger, "Me llegaron %d bytes con el mensaje: %s", bytesRecibidos, buffer);
		if(!strcmp(buffer,"Hola soy Memoria")){
			send(cliente, "Hola Memoria!", sizeof("Hola Memoria!"), 0);
			free(buffer);
			break;
		}
		free(buffer);
	}

	while(1){
		char* buffer = malloc(100);

		int bytesRecibidos = recv(cliente, buffer, 100, 0);
		if(bytesRecibidos < 0){
			log_error(logger, "El cliente se desconecto");
			exit(EXIT_FAILURE); ///
		}
		buffer[bytesRecibidos-1] = '\0';
		string_trim(&buffer);
		log_trace(logger, "Me llegaron %d bytes con el mensaje: %s", bytesRecibidos, buffer);
		char* resultado = apiLissandra(buffer);
		send(cliente, resultado, string_length(resultado), 0);
		free(buffer);
		free(resultado);
	}
}

// ############### Extras ###############

t_config* leer_config() {
	return config_create("LFS.config");
}

t_log* iniciar_logger() {
	return log_create("Lissandra.log", "Lissandra", 1, LOG_LEVEL_TRACE);
}

