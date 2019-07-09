#include "Kernel.h"
#include "ApiKernel.h"
#include "IPC.h"
#include "Memorias.h"
#include "Metricas.h"
#include "Misc.h"

sem_t new;
sem_t ready;
sem_t finish;
sem_t multiProc;

t_queue* colaNew;
t_queue* colaReady;
t_queue* colaFinish;


void inicializarColas(){
	colaNew = queue_create();
	colaReady = queue_create();
	colaFinish = queue_create();
}

void inicializarListasDeMemorias(){
	listasMemorias[SHC]= list_create();
	listasMemorias[SC]= list_create();
	listasMemorias[EC]= list_create();
}

void leerConfig();// Esta no se si esta demas xd .

void consola();
t_resultado apiKernel(char*);

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
	config.retardo_gossiping = config_get_int_value(configk,"RETARDO_GOSSIPING");
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

t_resultado apiKernel(char* mensaje){
	char** comando = string_split(mensaje, " "); // Separa el mensaje por espacios " ", y los mete en un array de char.
	if(comando[0]){
		u_int16_t cantArgumentos = 0;
		while(comando[cantArgumentos+1]){
			cantArgumentos++;
		}

		if(!strcmp(comando[0],"SELECT")){
			//SELECT [NOMBRE_TABLA] [KEY]
			//SELECT TABLA1 3
			t_resultado respuesta;
			free(comando[0]);
			if(cantArgumentos == 2){
				char* nombreTabla = comando[1];
				char* keystr = comando[2];
				char* endptr;
				ulong key = strtoul(keystr, &endptr, 10); //String To Unsigned Long, le paso un String y me retorna un ulong con ese numero.
				if(*endptr == '\0'&& key < 65536){        // Atoi era otra opcion pero no maneja errores como strtoul o strtol
														  // Como deben ser Keys de 16 bits debe ser < 65536

					respuesta = selects(nombreTabla, key);

					free(nombreTabla);
					free(keystr);
					free(comando);
					return respuesta;
				}
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			respuesta.falla = true;
			respuesta.resultado = strdup("Sintaxis invalida. Uso: SELECT [NOMBRE_TABLA] [KEY]");
			return respuesta;
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
						t_resultado resultado = insert(nombreTabla, key, valor);
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
			t_resultado resultado;
			resultado.falla = true;
			resultado.resultado = string_from_format("Sintaxis invalida. Uso: INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”");
			return resultado;
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
				else{
					t_resultado resultado;
					resultado.falla = true;
					resultado.resultado = string_from_format("Tipo de consistencia invalido.");
					return resultado;
				}

				char* cantidadParticionesstr = comando[3];
				char* compactionTimestr = comando[4];
				char* endptr = 0;
				ulong cantidadParticiones = strtoul(cantidadParticionesstr, &endptr, 10);
				ulong compactionTime;
				if(*endptr == '\0')
					compactionTime = strtoul(compactionTimestr, &endptr, 10);
				if(*endptr == '\0'){
					// Faltaria revisar si el tipo de consistencia es valido ^
					t_resultado resultado = create(nombreTabla, consistencia, cantidadParticiones, compactionTime);
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
			t_resultado resultado;
			resultado.falla = true;
			resultado.resultado = string_from_format("Sintaxis invalida. Uso: CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]");
			return resultado;
		}
		else if(!strcmp(comando[0],"DESCRIBE")){
			//DESCRIBE [NOMBRE_TABLA]
			//DESCRIBE TABLA1

			free(comando[0]);
			if(cantArgumentos == 1){
				char* nombreTabla = comando[1];
				t_resultado resultado = describe(nombreTabla);
				free(nombreTabla);
				free(comando);
				return resultado;
			}
			if(cantArgumentos == 0){
				t_resultado resultado = describe_global();
				free(comando);
				return resultado;
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			t_resultado resultado;
			resultado.falla = true;
			resultado.resultado = string_from_format("Sintaxis invalida. Uso: DESCRIBE [NOMBRE_TABLA]");
			return resultado;
		}
		else if(!strcmp(comando[0],"DROP")){
			//DROP [NOMBRE_TABLA]
			//DROP TABLA1

			free(comando[0]);
			if(cantArgumentos == 1){
				char* nombreTabla = comando[1];
				t_resultado resultado = drop(nombreTabla);
				free(nombreTabla);
				free(comando);
				return resultado;
			}
			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}
			free(comando);
			t_resultado resultado;
			resultado.falla = true;
			resultado.resultado = string_from_format("Sintaxis invalida. Uso: DROP [NOMBRE_TABLA]");
			return resultado;
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
			t_resultado resultado;
			resultado.falla = true;
			resultado.resultado = string_from_format("Sintaxis invalida. Uso: JOURNAL");
			return resultado;
		}

		else if(! strcmp(comando[0],"RUN")){
			//RUN <path> -> ruta del archivo LQL que se encuentra en el FileSystem local del Kernel y No en el proceso LFS del tp.
			free(comando[0]);
			if (cantArgumentos == 1)
			{
				char* rutaPath = comando[1];
				t_resultado resultado = run(rutaPath);
				free(rutaPath);
				free(comando);
				return resultado;
			}

			while(cantArgumentos){
				free(comando[cantArgumentos]);
				cantArgumentos--;
			}

			free(comando);
			t_resultado resultado;
			resultado.falla = true;
			resultado.resultado = string_from_format("Sintaxis invalida. Uso: RUN <PATH>");
			return resultado;

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
			t_resultado resultado;
			resultado.falla = true;
			resultado.resultado = string_from_format("Sintaxis invalida. Uso: METRICS");
			return resultado;

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
				else{
					t_resultado resultado;
					resultado.falla = true;
					resultado.resultado = string_from_format("Tipo de consistencia invalido.");
					return resultado;
				}

				char* endptr = 0;
				ulong numeroMemoria = strtoul(numerostr, &endptr, 10);
				if(*endptr == '\0'&& numeroMemoria < 65536){
					t_resultado resultado = add(numeroMemoria,criterio);
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
			t_resultado resultado;
			resultado.falla = true;
			resultado.resultado = string_from_format("Sintaxis invalida. Uso: ADD MEMORY [NUMERO] TO [CRITERIO]");
			return resultado;
		}


		while(cantArgumentos){
			free(comando[cantArgumentos]);
			cantArgumentos--;
		}
		free(comando[0]);
	}
	free(comando);
	t_resultado resultado;
	resultado.falla = true;
	resultado.resultado = string_from_format("Comando invalido");
	return resultado;

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
	bool fallo = false;
	for (int q = config.quantum; q > 0 && !queue_is_empty(requests) && !fallo; q--){
		if(q < config.quantum)
			usleep(config.retardo_ciclico * 1000); // Agregamos el retardo solo entre operaciones

		char* request = queue_pop(requests);
		t_resultado resultado = apiKernel(request);
		if(resultado.falla){
			log_warning(logger, "La request %s fallo con resultado: %s. Abortando la ejecucion del script.", request, resultado.resultado);
			fallo = true;
		}
		else{
			log_info(logger, "La request %s retorno como resultado: %s", request, resultado.resultado);
		}
		free(request);
		free(resultado.resultado);
	}

	// Si finalizo su ejecucion (por fin de script o fallo), va a la cola de terminados. Sino vuelve a la cola de listos.
	if(!queue_is_empty(requests) && !fallo){
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
		pthread_attr_t attr;                                //Para darle atributos al hilo: Que sea detachable
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
	}
} //End Corto Plazo

int main(void) {
	logger = log_create ("Kernel.log", "Kernel", 1 ,LOG_LEVEL_TRACE);
	log_info (logger, "Hola soy Kernel \n");

	leerConfig();
	log_info (logger, "Pude leer config ! \n");

	initMetricas();

	//Inicializo semaforos

	sem_init(&new,0,0);
	sem_init(&ready,0,0);
	sem_init(&finish,0,0);
	sem_init(&multiProc,0,config.multiprocesamiento);

	//Inicializo colas de estados

	inicializarColas();
	inicializarListasDeMemorias();

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
	//End Main
}

