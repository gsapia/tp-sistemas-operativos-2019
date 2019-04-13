/*
-Es el punto de entrada al LFS, administra la conexión con los demás módulos y  entiende como resolver los distintos pedidos que se le soliciten.

##### TABLAS #####
-Se divide a las tablas en particiones. Cada particion es un archivo en el FS. Cant de particiones? Definido en el Metadata.
-Cada tabla estara representada con un directorio, que contiene todos los archivos metadata (que corresponden a c/u de las particiones), ademas de temer un archivo de Metadata (propio)
-Tipos de archivos: (.bin - .tmp - tmpc


##### METADATA #####
-Contiene la informacion administrativa de la tabla.
	-Tipo de consistencia
	-Numero de Particiones
	-Tiempo entre compactaciones.

##### REGISTRO #####
-Los datos de la tabla estarán formados por registros, donde cada uno de ellos estará compuesto por 3 partes:
	-Timestamp (tiempo de la insercion de la key)
	-Key ( ... la key)
	-Value ( Con valor maximo en .config, tiene que controlar que no se hagan insert de tamaño mayor al maximo.)

¡Para la distribución de las distintas Key dentro de dicha tabla se aplicará una función módulo!
¡Se dividirá la key por la cantidad de particiones y el resto de la operación será la partición a utilizar!

##### .TMP / DUMPS / MEMTABLE
-.tmp = se guardan los datos que vayan modificandose para luego compactarlos
-Dos jerarquias de .tmp:
	-El area de "memtable", dividido por tabla, son los datos que no fueron dumpeados en un archivo temporal. Son los Insert que se acumulan hasta que se haga un DUMP.
	-Datos en .tmp que estan pendientes de compactacion

*/

#include "LFS.h"

void *consola();
void *apiLissandra();
void dump();
void funcionModulo();
t_config *leer_config();
int iniciar_servidor(char* PUERTO_ESCUCHA);
t_log* iniciar_logger();
int esperar_cliente(int socket_servidor);
void enviar_mensaje(char* mensaje, int socket_cliente);
void hiloAPILissandra(pthread_t hiloAPI);

int main(void){
	//Inicio todas las variables


/*
	t_config* config = leer_config();

	Hago de Servidor
	char* puertoEscucha = config_get_string_value(config, "PUERTOESCUCHA");
	int server_fd = iniciar_servidor(puertoEscucha);
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);

	//Envio el value a MEMORIA

	char* tamanoValue = config_get_string_value(config, "TAMAÑOVALUE");
	enviar_mensaje(tamanoValue, cliente_fd);
	*/
	//Inicio la API

	logger = iniciar_logger();
	log_info(logger, "Hola, soy Lissandra");

	pthread_t hiloConsola;
	int hilo = pthread_create(&hiloConsola, NULL, consola, NULL);
	if(hilo){
			fprintf(stderr,"Error - pthread_create() return code: %d\n",hilo);
			exit(EXIT_FAILURE);
		 }
		pthread_join(hiloConsola, NULL);


	//Esperar con recv() que MEMORIA nos mande las cosas


	return EXIT_SUCCESS;
}
