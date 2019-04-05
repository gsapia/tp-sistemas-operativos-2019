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
	-El area de "memtable", dividido por tabla, son los datos que no fueron dumpeados en un archivo temporal.
	-Datos en .tmp que estan pendientes de compactacion

*/

#include "LFS.h"

void *apiLissandra();
char *consola();
void dump();
void funcionModulo();


int main(void){
	pthread_t hiloAPI; int hilo;
	Registro registro;

	hilo = pthread_create(&hiloAPI, NULL, apiLissandra, NULL);
	if(hilo){
		fprintf(stderr,"Error - pthread_create() return code: %d\n",hilo);
		exit(EXIT_FAILURE);
	 }

	pthread_join(hiloAPI, NULL);

	dump();
	funcionModulo();
	return EXIT_SUCCESS;
}

void *apiLissandra(){
	char *linea;
	printf("Estas es la API de Lissandra.\n");

	while(1){
		linea = consola();
		if(!strncmp(linea,SELECT,6)){
			printf("Elegiste Select\n");
			//SELECT [NOMBRE_TABLA] [KEY]
			//SELECT TABLA1 3
			free(linea);
		}
		if(!strncmp(linea,INSERT,6)){
			printf("Elegiste Insert\n");
			//INSERT [NOMBRE_TABLA] [KEY] “[VALUE]” [Timestamp]
			//INSERT TABLA1 3 “Mi nombre es Lissandra” 1548421507
			free(linea);
		}
		if(!strncmp(linea,CREATE,6)){
			printf("Elegiste Create\n");
			//CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
			//CREATE TABLA1 SC 4 60000
			free(linea);
		}
		if(!strncmp(linea,DESCRIBE,8)){
			printf("Elegiste Describe\n");
			//DESCRIBE [NOMBRE_TABLA]
			//DESCRIBE TABLA1
			free(linea);
		}
		if(!strncmp(linea,DROP,4)){
			printf("Elegiste Drop\n");
			//DROP [NOMBRE_TABLA]
			//DROP TABLA1
			free(linea);
		}
		if(!strncmp(linea,EXIT, 4)) {
			printf("Elegiste Exit\n");
			free(linea);
			break;
		}
	}
}

char *consola(){
	char *linea;
	printf("Elegi la opcion que quieras que se ejecute:\n");
	printf("1.Select\n");
	printf("2.Insert\n");
	printf("3.Create\n");
	printf("4.Describe\n");
	printf("5.Drop\n");
	printf("0.Exit\n");
	linea = readline(">");
	return linea;
}

//Descarga toda la informacion de la memtable, de todas las tablas, y copia dichos datos en los ditintos archivos temporales (uno por tabla).
//Luego se limpia la memtable.
void dump(){
	printf("Hago Dump\n");
}
//Distribuye las disitntas Key dentro de dicha tabla.
//Se dividirá la key por la cantidad de particiones y el resto de la operación será la partición a utilizar
void funcionModulo(){
	printf("Hago Funcion Modulo\n");
}
