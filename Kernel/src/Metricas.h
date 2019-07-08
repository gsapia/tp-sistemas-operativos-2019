#ifndef METRICAS_H_
#define METRICAS_H_

#include <pthread.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>

typedef struct{
	unsigned long long read_latency; // El tiempo promedio que tarda un SELECT en ejecutarse en los últimos 30 segundos.
	unsigned long long write_latency; // El tiempo promedio que tarda un INSERT en ejecutarse en los últimos 30 segundos.
	unsigned int reads; // Cantidad de SELECT ejecutados en los últimos 30 segundos.
	unsigned int writes; // Cantidad de INSERT ejecutados en los últimos 30 segundos.
	t_dictionary* memory_load; // (por cada memoria):  Cantidad de INSERT / SELECT que se ejecutaron en esa memoria respecto de las operaciones totales.
}t_metricas;

typedef struct{
	unsigned long long inicio;
	unsigned long long fin;
}t_metrica_operacion;

t_queue * metricas_selects; // Metricas de los ultimos 30 seg
t_queue * metricas_inserts; // Metricas de los ultimos 30 seg

pthread_mutex_t mutex_metricas;

unsigned int selects_totales;
unsigned int inserts_totales;

void initMetricas(void);

void informar_select(unsigned long long inicio);
void informar_insert(unsigned long long inicio);

t_metricas get_metricas(void);

#endif /* METRICAS_H_ */
