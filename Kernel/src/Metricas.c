#include "Metricas.h"
#include "Kernel.h"
#include "ApiKernel.h"
#include "Misc.h"

t_metricas * metricas_respuesta;

void informar_metricas(){ // Informa las metricas cada 30 seg
	char* resultado;
	while(1){
		sleep(5);
		resultado = metrics().resultado;
		log_info(logger, "%s", resultado);
		free(resultado);
	}
}

void initMetricas(void){
	metricas_respuesta = calloc(3, sizeof(t_metricas));
	for (int consistencia = 0; consistencia < 3; ++consistencia) {
		metricas[consistencia].ultimos_selects = queue_create();
		metricas[consistencia].ultimos_inserts = queue_create();
		metricas[consistencia].operaciones_totales = 0;
		metricas[consistencia].operaciones_por_memoria = dictionary_create();

		metricas_respuesta[consistencia].memory_load = dictionary_create();
	}

	pthread_t hiloMetricas;
	pthread_create(&hiloMetricas, NULL,(void*)informar_metricas , NULL);
	pthread_detach(hiloMetricas);
}

// Limpia las metricas mayores a 30 segundos atras
void limpiar_metricas(void){
	unsigned long long timestamp_30seg = getTimestamp() - 30000; // El timestamp de hace 30 segundos atras
	for (int consistencia = 0; consistencia < 3; ++consistencia) {
		t_queue * metricas_selects = metricas[consistencia].ultimos_selects;
		while(!queue_is_empty(metricas_selects) && ((t_metrica_operacion*)queue_peek(metricas_selects))->fin < timestamp_30seg){
			free(queue_pop(metricas_selects));
		}
		t_queue * metricas_inserts = metricas[consistencia].ultimos_inserts;
		while(!queue_is_empty(metricas_inserts) && ((t_metrica_operacion*)queue_peek(metricas_inserts))->fin < timestamp_30seg){
			free(queue_pop(metricas_inserts));
		}
	}
}

void agregar_metrica(t_metrica_operacion metrica, t_queue* cola){
	t_metrica_operacion* aux = malloc(sizeof(metrica));
	*aux = metrica;

	queue_push(cola, aux);

	limpiar_metricas();
}

void informar_select(enum consistencias consistencia, unsigned int nro_memoria, unsigned long long inicio){
	t_metrica_operacion metrica = { .inicio = inicio, .fin = getTimestamp() };
	pthread_mutex_lock(&mutex_metricas);
	agregar_metrica(metrica, metricas[consistencia].ultimos_selects);
	metricas[consistencia].operaciones_totales++;

	unsigned int * operaciones_memoria;
	char* memoria = string_from_format("%d", nro_memoria); // Uso char* nomas porque el diccionario de la commons es asi.
	if(dictionary_has_key(metricas[consistencia].operaciones_por_memoria, memoria)){ // Si hay un valor para esa memoria, simplemente lo actualizamos. Sino, lo agregamos.
		operaciones_memoria = dictionary_get(metricas[consistencia].operaciones_por_memoria, memoria);
		(*operaciones_memoria)++;
	}
	else{
		operaciones_memoria = malloc(sizeof(operaciones_memoria));
		*operaciones_memoria = 1;
		dictionary_put(metricas[consistencia].operaciones_por_memoria, memoria, operaciones_memoria);
	}

	pthread_mutex_unlock(&mutex_metricas);
}
void informar_insert(enum consistencias consistencia, unsigned int nro_memoria, unsigned long long inicio){
	t_metrica_operacion metrica = { .inicio = inicio, .fin = getTimestamp() };
	pthread_mutex_lock(&mutex_metricas);
	agregar_metrica(metrica, metricas[consistencia].ultimos_inserts);
	metricas[consistencia].operaciones_totales++;

	unsigned int * operaciones_memoria;
	char* memoria = string_from_format("%d", nro_memoria); // Uso char* nomas porque el diccionario de la commons es asi.
	if(dictionary_has_key(metricas[consistencia].operaciones_por_memoria, memoria)){ // Si hay un valor para esa memoria, simplemente lo actualizamos. Sino, lo agregamos.
		operaciones_memoria = dictionary_get(metricas[consistencia].operaciones_por_memoria, memoria);
		(*operaciones_memoria)++;
	}
	else{
		operaciones_memoria = malloc(sizeof(operaciones_memoria));
		*operaciones_memoria = 1;
		dictionary_put(metricas[consistencia].operaciones_por_memoria, memoria, operaciones_memoria);
	}

	pthread_mutex_unlock(&mutex_metricas);
}

unsigned long long get_latencias(t_queue* metricas){
	if(queue_is_empty(metricas))
		return 0;

	unsigned long long latencia = 0;

	void iterador(t_metrica_operacion * metrica){
		latencia += metrica->fin - metrica->inicio;
	}

	list_iterate(metricas->elements, (void (*)(void*))iterador);
	return latencia / queue_size(metricas);
}

t_metricas * get_metricas(void){
	pthread_mutex_lock(&mutex_metricas);

	limpiar_metricas();

	for (int consistencia = 0; consistencia < 3; ++consistencia) {
		t_queue * ultimos_selects = metricas[consistencia].ultimos_selects;
		t_queue * ultimos_inserts = metricas[consistencia].ultimos_inserts;

		metricas_respuesta[consistencia].read_latency = get_latencias(ultimos_selects);
		metricas_respuesta[consistencia].write_latency = get_latencias(ultimos_inserts);

		metricas_respuesta[consistencia].reads = queue_size(ultimos_selects);
		metricas_respuesta[consistencia].writes = queue_size(ultimos_inserts);

		void iterador(t_memoria* memoria){
			char* nro_memoria = string_from_format("%d", memoria->numero);

			float * memory_load;
			if(dictionary_has_key(metricas_respuesta[consistencia].memory_load, nro_memoria)){
				memory_load = dictionary_get(metricas_respuesta[consistencia].memory_load, nro_memoria);
			}
			else{
				memory_load = malloc(sizeof(memory_load));
				dictionary_put(metricas_respuesta[consistencia].memory_load, strdup(nro_memoria), memory_load);
			}

			if(dictionary_has_key(metricas[consistencia].operaciones_por_memoria, nro_memoria)){
				unsigned int * operaciones = dictionary_get(metricas[consistencia].operaciones_por_memoria, nro_memoria);
				*memory_load = ((float)(*operaciones)) / metricas[consistencia].operaciones_totales;
			}
			else{
				*memory_load = 0;
			}

			free(nro_memoria);
		}

		list_iterate(listasMemorias[consistencia], (void(*)(void*)) iterador);
	}

	pthread_mutex_unlock(&mutex_metricas);

	return metricas_respuesta;
}
