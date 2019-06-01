#include "Kernel.h"


//Consola

    void* consola();
    char* apiKernel(char*);


     void* consola(){
		char *linea;
		char *resultado;
		while(1) {
			linea = readline(">");

			if(!strcmp(linea,"exit")){
				free(linea);
				break;
			}

			resultado = apiKernel(linea);
			free(linea);
			puts(resultado);
			free(resultado);
		}

	}

	//API Kernel

	char *apiKernel(char* mensaje){
		char** comando = string_split(mensaje, " "); // Separa el mensaje por espacios " ", y los mete en un array de char.
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
					ulong key = strtoul(keystr, &endptr, 10); //String To Unsigned Long, le paso un String y me retorna un ulong con ese numero.
					if(*endptr == '\0'&& key < 65536){       // Atoi era otra opcion pero no maneja errores como strtoul o strtol
						char* resultado = selects(nombreTabla, key); // Como deben ser Keys de 16 bits debe ser < 65536
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
			else if(!strcmp(comando[0],"INSERT")){
				//INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
				//INSERT TABLA1 3 "Mi nombre es Lissandra"

				free(comando[0]);
				if (cantArgumentos >= 3) {
					char** argumentos = string_n_split(mensaje, 4, " "); // Separa el mensaje por " ", y los mete en un array de 4 espacios, el ultimo es NULL
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
			else if(!strcmp(comando[0],"DESCRIBE")){
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

			else if(!strcmp(comando[0],"JOURNAL")){
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

			else if(! strcmp(comando[0],RUN)){
				//RUN <path> -> ruta del archivo LQL que se encuentra en el FileSystem local del Kernel y No en el proceso LFS del tp.
				free(comando[0]);
				if (cantArgumentos == 1)
				{
					char* rutaPath = comando[1];
					char* resultado = run(rutaPath);
					free(rutaPath);
					free(comando);
					return resultado;
				}

				while(cantArgumentos){
					free(comando[cantArgumentos]);
					cantArgumentos--;
				}

				free(comando);
				return string_from_format("Sintaxis invalida. Uso: RUN <PATH>");

			}

			else if(! strcmp(comando[0], "METRICS")){
				//METRICS Informa las metricas actuales por consola
				free(comando[0]);
				if (cantArgumentos == 0)
				{
					free(comando);
					return metrics();
				}

				while(cantArgumentos){
					free(comando[cantArgumentos]);
					cantArgumentos--;
				}

				free(comando);
				return string_from_format("Sintaxis invalida. Uso: METRICS");

			}

			else if( (! strcmp(comando[0],"ADD") ) && ((! strcmp(comando[1],"MEMORY") ))  ){
				// ADD MEMORY [NUMERO] TO [CRITERIO]
				// ADD MEMORY 3 TO SC
				free(comando[0]);
				free(comando[1]);
				if( cantArgumentos == 3 && (!strcmp(comando[3],"TO")) ){
					char* numerostr = comando[2];
					char* criterio = comando[4];
					char* endptr = 0;
					ulong numeroMemoria = strtoul(numeroMemoria, &endptr, 10);
					if(*endptr == '\0'&& numeroMemoria < 65536){
						char* resultado = add(numeroMemoria,criterio);
						free(numerostr);
						free(criterio);
						free(comando);
					}

				}
				while(cantArgumentos){
					free(comando[cantArgumentos]);
					cantArgumentos--;
				}
				free(comando);
				return string_from_format("Sintaxis invalida. Uso: ADD");
			}


			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando[0]);


		free(comando);
		return string_from_format("Comando invalido");

		}


	} //end Char* apiKernel ..





	void saludar(){
			printf("Hola soy un hilo \n");
				}

		void despedir()
		{
			printf("Adios hilo!\n");
		}


int main(void) {


	logger = log_create ("Kernel.log", "Kernel", 1 ,LOG_LEVEL_TRACE);
    log_info (logger, "Hola soy KKernel \n");

    printf ( "Prueba de hilo \n" );



 /*
    pthread_t hiloAConsola;
	if(pthread_create(&hiloAConsola, NULL,(void*)consola , NULL))
	{
		log_error(logger,"Hilo consola: error en la creacion pthread_create");
		exit(EXIT_FAILURE);
	}

	*/


/*

	// Algoritmo Plani RR SuperIncompleto


	int quantum= 4; // Aca deberia ir "QUANTUM" .
	int contador,cantScripts,time,remain,flag=0;
	int tiempoEspera =0, tiempoRetorno=0, arrivosVec[10],rafagasVec[10],rt[10];
	//printf("Ingrese la cantidad de scripts");
	//scanf ("%d",&cantScripts);
	remain = cantScripts;

	for (contador=0; contador < cantScripts ; contador++)
	{
		printf("Ingrese los arribos y rafagas del script %d :", contador +1 );
		scanf("%d", &arrivosVec[contador]);
		scanf("%d", &rafagasVec[contador]);
		rt[contador]=rafagasVec[contador];
	}


	for(time=0, contador=0; remain!=0;)
	{
		if(rt[contador] <= quantum && rt[contador]>0)
		{
		time =+ rafagasVec[contador];
		rafagasVec[contador]=0;
		flag =1;

		}//end if
		else if(rt[contador]>0)
		{
			rt[contador]-=quantum;
			time+=quantum;
		}
		if(rt[contador]==0 && flag == 1)
		{
			remain--;
			      printf("P[%d]\t|\t%d\t|\t%d\n",contador+1,time-arrivosVec[contador],time-arrivosVec[contador]-rafagasVec[contador]);
			      tiempoEspera+=time-arrivosVec[contador]-rafagasVec[contador];
			      tiempoRetorno+=time-arrivosVec[contador];
			      flag=0;
		}
		if(contador==cantScripts-1)
		      contador=0;
		    else if(arrivosVec[contador+1]<=time)
		      contador++;
		    else
		      contador=0;
		  }
		  printf("\nAverage Waiting Time= %f\n",tiempoEspera*1.0/cantScripts);
		  printf("Avg Tiempo De Retorno Time = %f",tiempoRetorno*1.0/cantScripts);


	}// Fin RR







    //Pruebas de hilos

    pthread_t unHilo;
	pthread_create(&unHilo,NULL, (void*)saludar,NULL);

    pthread_join(unHilo,NULL);

    pthread_t otroHilo;
	pthread_create (&otroHilo, NULL, (void*)despedir,NULL);

    pthread_join (otroHilo,NULL);




*/



 //End Main

}

