//Aca van todas las funciones auxiliares que no sean inherentes del File System en si.
#include "LFS.h"

// ############### LISSANDRA ###############

void* consola();
char* apiLissandra(char*);

char* selects(char* nombreTabla, u_int16_t key);
// ^ La funcion select() ya existe, hay que buscar otro nombre mas creativo.... ^
char* insert(char* nombreTabla, u_int16_t key, char* valor);
char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
char* describe(char* nombreTabla);
char* drop(char* nombreTabla);


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
	log_debug(logger, "SELECT: Recibi Tabla: %s Key: %d", nombreTabla, key);
	return string_duplicate("Elegiste SELECT");
}
char* insert(char* nombreTabla, u_int16_t key, char* valor){
	log_debug(logger, "INSERT: Recibi Tabla: %s Key:%d Valor: %s", nombreTabla, key, valor);
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

// ############### SOCKET SERVIDOR ###############

void* servidor(int puerto_escucha){
	log_trace(logger, "Iniciando servidor");

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons((uint16_t)puerto_escucha);

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
		char* buffer = malloc(17);

		int bytesRecibidos = recv(cliente, buffer, 16, 0);
		if(bytesRecibidos < 0){
			log_error(logger, "El cliente se desconecto");
			exit(EXIT_FAILURE);
		}
		buffer[bytesRecibidos] = '\0';
		string_trim(&buffer);
		log_trace(logger, "Me llego el mensaje: %s", buffer);
		if(!strcmp(buffer,"Hola soy Memoria")){
			send(cliente, "Hola Memoria!", sizeof("Hola Memoria!"), 0);
			free(buffer);
			break;
		}
		free(buffer);
	}

	while(1){
		char* buffer = malloc(100);

		int bytesRecibidos = recv(cliente, buffer, 99, 0);
		if(bytesRecibidos < 0){
			log_error(logger, "El cliente se desconecto");
			exit(EXIT_FAILURE); ///
		}
		buffer[bytesRecibidos-1] = '\0';
		string_trim(&buffer);
		log_trace(logger, "Me llego el mensaje: %s", buffer);
		char* resultado = apiLissandra(buffer);
		send(cliente, resultado, string_length(resultado), 0);
		free(buffer);
		free(resultado);
	}
}

/*
int iniciar_servidor(char* PUERTO_ESCUCHA)
{

	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, PUERTO_ESCUCHA, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    log_trace(logger, "Listo para escuchar a mi cliente");

    return socket_servidor;
}
int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}
int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}
void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}
void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}
//podemos usar la lista de valores para poder hablar del for y de como recorrer la lista
t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

*/
// ############### Extras ###############

t_config* leer_config() {
	return config_create("LFS.config");
}

t_log* iniciar_logger() {
	return log_create("Lissandra.log", "Lissandra", 1, LOG_LEVEL_TRACE);
}
