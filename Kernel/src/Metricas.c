#include "Metricas.h"
#include "Kernel.h"
#include "ApiKernel.h"
#include "Misc.h"

void informar_metricas(){ // Informa las metricas cada 30 seg
	while(1){
		sleep(30);
		log_info(logger, metrics().resultado);
	}
}

void initMetricas(void){
	metricas_selects = queue_create();
	metricas_inserts = queue_create();

	pthread_t hiloMetricas;
	pthread_create(&hiloMetricas, NULL,(void*)informar_metricas , NULL);
	pthread_detach(hiloMetricas);
}

// Limpia las metricas mayores a 30 segundos atras
void limpiar_metricas(void){
	unsigned long long timestamp_30seg = getTimestamp() - 30000; // El timestamp de hace 30 segundos atras
	while(!queue_is_empty(metricas_selects) && ((t_metrica_operacion*)queue_peek(metricas_selects))->fin < timestamp_30seg){
		free(queue_pop(metricas_selects));
	}
	while(!queue_is_empty(metricas_inserts) && ((t_metrica_operacion*)queue_peek(metricas_inserts))->fin < timestamp_30seg){
		free(queue_pop(metricas_inserts));
	}
}

void agregar_metrica(t_metrica_operacion metrica, t_queue* cola){
	t_metrica_operacion* aux = malloc(sizeof(metrica));
	*aux = metrica;

	queue_push(cola, aux);

	limpiar_metricas();
}

void informar_select(unsigned long long inicio){
	t_metrica_operacion metrica = { .inicio = inicio, .fin = getTimestamp() };
	pthread_mutex_lock(&mutex_metricas);
	agregar_metrica(metrica, metricas_selects);
	selects_totales++;
	pthread_mutex_unlock(&mutex_metricas);
}
void informar_insert(unsigned long long inicio){
	t_metrica_operacion metrica = { .inicio = inicio, .fin = getTimestamp() };
	pthread_mutex_lock(&mutex_metricas);
	agregar_metrica(metrica, metricas_inserts);
	inserts_totales++;
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

t_metricas get_metricas(void){
	t_metricas metricas;
	pthread_mutex_lock(&mutex_metricas);

	limpiar_metricas();

	metricas.read_latency = get_latencias(metricas_selects);
	metricas.write_latency = get_latencias(metricas_inserts);

	metricas.reads = selects_totales;
	metricas.writes = inserts_totales;

	// TODO Memory load

	pthread_mutex_unlock(&mutex_metricas);

	return metricas;
}
