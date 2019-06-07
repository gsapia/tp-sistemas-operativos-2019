#include "Kernel.h"
#include "ApiKernel.h"
#include "IPC.h"

sem_t new;
sem_t ready;
sem_t exec;
sem_t finish;
sem_t multiProc;

t_queue* colaNew;
t_queue* colaReady;
t_queue* colaExec;
t_queue* colaFinish;


void inicializarColas(){
	colaNew = queue_create();
	colaReady = queue_create();
	colaExec = queue_create();
	colaFinish = queue_create();
}


 void leerConfig();// Esta no se si esta demas xd .

    void consola();
    char* apiKernel(char*);

    //Levanto la configuracion
    t_config* configk;
    void leerConfig(){

    	configk = config_create("Kernel.config");

    	config.ip_memoria = config_get_string_value(configk,"IP_MEMORIA");
    	config.puerto_memoria = config_get_int_value(configk,"PUERTO_MEMORIA");
    	config.quantum = config_get_int_value(configk,"QUANTUM");
    	config.multiprocesamiento = config_get_int_value(configk,"MULTIPROCESAMIENTO");
    	config.refresh_metadata = config_get_int_value(configk,"REFRESH_METADATA");
    	config.retardo_ciclico = config_get_int_value(configk,"RETARDO_CICLICO");

    }

    void aniadirScript(t_script *script){
		queue_push(colaNew, script);
		sem_post(&new);
    }

     //Consola Kernel

     void consola(){
		char *linea;
		while(1) {
			linea = readline(">");

			if(!strcmp(linea,"exit")){
				free(linea);
				break;
			}

			t_queue* requests = queue_create();
			queue_push(requests, linea);
			t_script *script = malloc(sizeof(t_script));
			script->nombre = strdup("Request por consola");
			script->requests = requests;
			aniadirScript(script);
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
					char* tipoConsistenciaStr = comando[2];

					enum consistencias consistencia;    // Declaro "criterio" de tipo consistencias sacado de la serializacion.h
					// para luego saber de que tipo es con un if debajo..

					if(!strcmp(tipoConsistenciaStr, "SC"))
						consistencia = SC;
					else if(!strcmp(tipoConsistenciaStr, "SHC"))
						consistencia = SHC;
					else if(!strcmp(tipoConsistenciaStr, "EC"))
						consistencia = EC;
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
						// Faltaria revisar si el tipo de consistencia es valido ^
						char* resultado = create(nombreTabla, consistencia, cantidadParticiones, compactionTime);
						free(nombreTabla);
						free(tipoConsistenciaStr);
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

			else if(! strcmp(comando[0],"RUN")){
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

			else if( (! strcmp(comando[0],"ADD") ) && (cantArgumentos > 0 && (! strcmp(comando[1],"MEMORY") ))  ){
				// ADD MEMORY [NUMERO] TO [CRITERIO]
				// ADD MEMORY 3 TO SC
				free(comando[0]);
				free(comando[1]);
				if( cantArgumentos == 4 && (!strcmp(comando[3],"TO")) ){

					char* numerostr = comando[2];
					char* criterioStr = comando[4]; //Agarro el criterio como un string

					enum consistencias criterio;    // Declaro "criterio" de tipo consistencias sacado de la serializacion.h
					                                // para luego saber de que tipo es con un if debajo..

					if(!strcmp(criterioStr, "SC"))
						criterio = SC;
					else if(!strcmp(criterioStr, "SHC"))
						criterio = SHC;
					else if(!strcmp(criterioStr, "EC"))
						criterio = EC;
					else

						return string_from_format("Tipo de consistencia invalido.");

					char* endptr = 0;
					ulong numeroMemoria = strtoul(numerostr, &endptr, 10);
					if(*endptr == '\0'&& numeroMemoria < 65536){
						char* resultado = add(numeroMemoria,criterio);
						free(numerostr);
						free(criterioStr);
						free(comando);
						return resultado;
					}

				}
				while(cantArgumentos){
					free(comando[cantArgumentos]);
					cantArgumentos--;
				}
				free(comando);
				return string_from_format("Sintaxis invalida. Uso: ADD MEMORY [NUMERO] TO [CRITERIO]");
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

	void terminarScripts(){
		while(1){
			sem_wait(&finish);
			t_script* finalizado = queue_pop(colaFinish);
			log_trace(logger, "Finalizó el script \"%s\"", finalizado->nombre);
			queue_destroy_and_destroy_elements(finalizado->requests, free);
			free(finalizado->nombre);
			free(finalizado);
		}
	}

	void largoPlazo(){
		// Creamos un hilo que se encargue de manejar la cola de terminados
		pthread_t hiloExit;
		if(pthread_create(&hiloExit, NULL,(void*)terminarScripts , NULL))
		{
			log_error(logger,"Hilo largo plazo: error en la creacion del hilo de exit");
			exit(EXIT_FAILURE);
		}
		pthread_detach(hiloExit);

		// Este hilo se encarga de la cola de nuevos nada mas
		while (1){
			sem_wait(&new);

			t_script* script = queue_pop(colaNew);

			queue_push(colaReady,script);
			//moverColaDeReady(script);

			sem_post(&ready);
		}

	} // End Largo Plazo


	void ejecutarScript(t_script* script){
		t_queue* requests = script->requests;   // busco las request con un puntero a la lista de las mismas
		for (int q = config.quantum; q > 0 && !queue_is_empty(requests); q--){
			char* request = queue_pop(requests);
			char* resultado = apiKernel(request);
			log_info(logger, "La request %s retorno como resultado: %s", request, resultado);

			free(request);
			free(resultado);

			usleep(config.retardo_ciclico * 1000);
		}

		// Si finalizo su ejecucion, va a la cola de terminados. Sino vuelve a la cola de listos.
		if(!queue_is_empty(requests)){
			queue_push(colaReady,script);
			sem_post(&ready);
			//moverAColaReady(scripts)
		}
		else{
			queue_push(colaFinish,script);
			sem_post(&finish);
		}
		sem_post(&multiProc);
	}//end ejecutarScript


	void cortoPlazo(){
		while (1){
			sem_wait(&ready);
			sem_wait(&multiProc);

			t_script* script = queue_pop(colaReady);

			// Mando la ejecucion a un hilo_exec deatacheable
			pthread_t hilo_exec;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			while(pthread_create(&hilo_exec, &attr, (void*)ejecutarScript, script)){
				log_info(logger, "Error creando hilo exec");
				queue_push(colaFinish,script);
				sem_post(&finish);
			}
			pthread_attr_destroy(&attr);

			//queue_push(colaExec,script);
			//moverAColaExec(script);

			sem_post(&exec);
		}
	} //End Corto Plazo






	void saludar(){
		printf("Hola soy un hilo \n");
	}

	void despedir()
	{
		printf("Adios hilo!\n");
	}


int main(void) {
	logger = log_create ("Kernel.log", "Kernel", 1 ,LOG_LEVEL_TRACE);
    log_info (logger, "Hola soy Kernel \n");

    leerConfig();
    log_info (logger, "Pude leer config ! \n");

	//Inicializo semaforos

	sem_init(&new,0,0);
	sem_init(&ready,0,0);
	sem_init(&exec,0,0);
	sem_init(&finish,0,0);
	sem_init(&multiProc,0,config.multiprocesamiento);

	//Inicializo colas de estados

	inicializarColas();

    printf ("Prueba de hilo \n");


    pthread_t hiloCliente;
   	if(pthread_create(&hiloCliente, NULL,(void*)initCliente , NULL))
   	{
   		log_error(logger,"Hilo cliente: error en la creacion pthread_create");
   		exit(EXIT_FAILURE);
   	}
    pthread_join(hiloCliente,NULL);

    // Iniciamos los planificadores
    pthread_t hiloLargoPlazo;
    if(pthread_create(&hiloLargoPlazo, NULL,(void*)largoPlazo , NULL))
    {
    	log_error(logger,"Hilo Largo Plazo: error en la creacion pthread_create");
    	exit(EXIT_FAILURE);
    }
    pthread_t hiloCortoPlazo;
    if(pthread_create(&hiloCortoPlazo, NULL,(void*)cortoPlazo , NULL))
    {
    	log_error(logger,"Hilo Corto Plazo: error en la creacion pthread_create");
    	exit(EXIT_FAILURE);
    }
    log_info(logger, "Planificadores iniciados.");



    pthread_t hiloAConsola;
	if(pthread_create(&hiloAConsola, NULL,(void*)consola , NULL))
	{
		log_error(logger,"Hilo consola: error en la creacion pthread_create");
		exit(EXIT_FAILURE);
	}

     pthread_join (hiloAConsola,NULL);

     log_destroy(logger); // Liberamos memoria del log
     config_destroy(configk); // Liberamos memoria de archivo de config

   /*  pthread_t hiloUpScript;
     if(pthread_create(&hiloUpScript,NULL,(void*)levantarScripts,NULL)){

    	 log_error(logger,"Hilo levantador de script: error en la creacion pthread_create");
    	 exit(EXIT_FAILURE);

     }
*/

/*

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

*/



/*

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

