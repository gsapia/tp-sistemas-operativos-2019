#include "Metricas.h"
#include "Kernel.h"
#include "ApiKernel.h"
#include "Misc.h"

void informar_metricas(){ // Informa las metricas cada 30 seg
	while(1){
		sleep(30);
		char* resultado = metrics().resultado;
		log_info(logger, resultado);
		free(resultado);
	}
}

void initMetricas(void){
	for (int consistencia = 0; consistencia < 3; ++consistencia) {
		metricas[consistencia].ultimos_selects = queue_create();
		metricas[consistencia].ultimos_inserts = queue_create();
		metricas[consistencia].operaciones_totales = 0;
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

void informar_select(enum consistencias consistencia, unsigned long long inicio){
	t_metrica_operacion metrica = { .inicio = inicio, .fin = getTimestamp() };
	pthread_mutex_lock(&mutex_metricas);
	agregar_metrica(metrica, metricas[consistencia].ultimos_selects);
	pthread_mutex_unlock(&mutex_metricas);
}
void informar_insert(enum consistencias consistencia, unsigned long long inicio){
	t_metrica_operacion metrica = { .inicio = inicio, .fin = getTimestamp() };
	pthread_mutex_lock(&mutex_metricas);
	agregar_metrica(metrica, metricas[consistencia].ultimos_inserts);
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
	t_metricas * metricas_respuesta = calloc(3, sizeof(t_metricas));
	pthread_mutex_lock(&mutex_metricas);

	limpiar_metricas();

	for (int consistencia = 0; consistencia < 3; ++consistencia) {
		t_queue * ultimos_selects = metricas[consistencia].ultimos_selects;
		t_queue * ultimos_inserts = metricas[consistencia].ultimos_inserts;

		metricas_respuesta[consistencia].read_latency = get_latencias(ultimos_selects);
		metricas_respuesta[consistencia].write_latency = get_latencias(ultimos_inserts);

		metricas_respuesta[consistencia].reads = queue_size(ultimos_selects);
		metricas_respuesta[consistencia].writes = queue_size(ultimos_inserts);

		// TODO Memory load
	}

	pthread_mutex_unlock(&mutex_metricas);

	return metricas_respuesta;
}
