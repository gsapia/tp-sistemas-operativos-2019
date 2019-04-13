//Aca van todas las funciones auxiliares que no sean inherentes del File System en si.
#include "LFS.h"

// ############### LISSANDRA ###############

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


void* consola(){
	char *linea;
	char *resultado;
	while(1) {
		linea = readline(">");

		if(!strcmp(linea,"exit")){
			free(linea);
			break;
		}

		resultado = apiMemoria(linea);
		free(linea);
		puts(resultado);
		free(resultado);
	}
}

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


//Descarga toda la informacion de la memtable, de todas las tablas, y copia dichos datos en los ditintos archivos temporales (uno por tabla). Luego se limpia la memtable.
void dump(){
	printf("Hago Dump\n");
}

//Distribuye las disitntas Key dentro de dicha tabla. Se dividirá la key por la cantidad de particiones y el resto de la operación será la partición a utilizar
int funcionModulo(int key, int particiones){
	return key % particiones;
}

// ############### SOCKET SERVIDOR ###############

int iniciar_servidor(char* PUERTO_ESCUCHA)
{

	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO_ESCUCHA, &hints, &servinfo);

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


// ############### SOCKET CLIENTE ###############

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_super_paquete(void)
{
	//me falta un malloc!
	t_paquete* paquete = malloc(sizeof(t_paquete));;

	//descomentar despues de arreglar
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}


// ############### Extras ###############

t_config* leer_config() {
	return config_create("LFS.config");
}

t_log* iniciar_logger() {
	return log_create("log.log", "LissandraServer", 1, LOG_LEVEL_DEBUG);
}
