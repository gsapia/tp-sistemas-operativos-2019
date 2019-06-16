#include "MemoriaPrincipal.h"
#include "Misc.h"

uint64_t* paginas_usadas; // Habria que modificar esto para usar LRU
bool full = false;

void auto_journaling(){
	while(1){
		msleep(config.tiempo_journal);
		vaciar_memoria();
	}
}

void initMemoriaPrincipal(){
	log_trace(logger, "Inicializando la memoria principal con un tamanio de %d bytes", config.tamanio_memoria);
	memoria_principal = malloc(config.tamanio_memoria);
	if(memoria_principal == NULL){
		log_error(logger, "No se pudo reservar espacio para la memoria principal. Hay suficiente RAM disponible?");
		exit(EXIT_FAILURE);
	}

	// Inicializo el tamanio de pagina
	tamanio_pagina = sizeof(uint64_t) + sizeof(uint16_t) + tamanio_value;
	log_trace(logger, "Tendremos un total de %d paginas de un tamanio de %d bytes cada una", CANT_PAGINAS, tamanio_pagina);

	// Inicializo la tabla de segmentos
	tabla_segmentos = list_create();

	paginas_usadas = calloc(CANT_PAGINAS, sizeof(*paginas_usadas));

	pthread_t hiloJournaling;
	if (pthread_create(&hiloJournaling, NULL, (void*)auto_journaling, NULL)) {
		log_error(logger, "Hilo auto_journaling: Error - pthread_create()");
		exit(EXIT_FAILURE);
	}
	pthread_detach(hiloJournaling);
}

// Devuelve el marco de una pagina que no haya sido modificada, o -1 en caso de no haber ninguna
int algoritmo_reemplazo(){
	log_trace(logger, "Ejecutando algoritmo de reemplazo.");
	int marco = -1;
	bool buscador_pagina_libre(t_pagina *pagina){
		return !pagina->modificado;
	}
	void iterador_pagina(t_segmento *segmento){
		if(marco == -1){
			t_list* tabla_paginas = segmento->paginas;
			t_pagina *pagina = list_remove_by_condition(tabla_paginas, (_Bool (*)(void*))buscador_pagina_libre);

			if (pagina) {
				// Encontramos una pagina que podemos reemplazar
				marco = pagina->numero;
				free(pagina);
				log_trace(logger, "Reemplazamos una pagina.");
			}
		}
	}
	list_iterate(tabla_segmentos, (void(*)(void*))iterador_pagina);

	if(marco == -1){
		log_trace(logger, "No hay paginas para reemplazar.");
	}

	return marco;
}

// SOLO USAR DESDE getPagina()
int getMarco(){
	int marco;
	for(marco = 0; marco < CANT_PAGINAS && *(paginas_usadas + marco); marco++);

	if(marco == CANT_PAGINAS){
		// Todas las paginas estan ocupadas, hay que intentar reemplazar
		marco = algoritmo_reemplazo();
		if(marco == -1){
			log_trace(logger, "La memoria esta FULL.");
			// TODO: Logica de memoria llena
			full = true;
			return -1;
		}
	}

	return marco;
}

// Devuelve una nueva pagina alojada ya en memoria
t_pagina* getPagina(){
	if(full) // Seamos eficientes, si la memoria esta FULL evitamos hacer nada
		return NULL;

	int marco = getMarco();
	if(marco >= 0)
		*(paginas_usadas + marco) = getTimestamp(); // Actualizo el timestamp
	if(marco == -1)
		return NULL;

	t_pagina *pagina = malloc(sizeof(t_pagina));

	pagina->marco = memoria_principal + (tamanio_pagina * marco);
	pagina->modificado = false;
	pagina->numero = marco;

	log_debug(logger, "Asignando el frame %d", marco);

	return pagina;
}

t_pagina* inicializar_nuevo_registro(char* nombreTabla, u_int16_t key, char* valor){
	// Busco si el segmento correspondiente existe en la tabla de segmentos
	t_segmento* segmento = buscar_segmento(nombreTabla);
	if(!segmento){
		// El segmento para esa tabla todavia no existe en la tabla de segmentos
		log_debug(logger, "Todavia no existe un segmento para esa tabla");

		// Agregamos el segmento para esa tabla
		segmento = agregar_segmento(nombreTabla);
	}
	// El segmento para esa tabla ya existe en la tabla de segmentos


	// Agrego el registro
	t_pagina* pagina = agregar_registro(key, valor, segmento);

	msleep(config.retardo_acc_mp);

	return pagina;
}

t_registro leer_registro(t_pagina* pagina){
	t_registro registro;
	registro.key = *((uint16_t*)(pagina->marco + DESPL_KEY));
	registro.valor = pagina->marco + DESPL_VALOR;
	registro.timestamp = *((uint64_t*)(pagina->marco + DESPL_TIMESTAMP));

	*(paginas_usadas + pagina->numero) = getTimestamp(); // Actualizo el timestamp

	msleep(config.retardo_acc_mp);

	return registro;
}

void modificar_registro(t_pagina* pagina, char* valor){
	t_marco marco = pagina->marco;
	pagina->modificado = true; // TODO: Posible condicion de carrera, el algoritmo de reemplazo podria reemplazar esta pagina justo antes de que cambiemos este flag

	uint64_t *timestamp = marco + DESPL_TIMESTAMP;
	char *value = marco + DESPL_VALOR;

	*timestamp = getTimestamp();
	strcpy(value, valor);

	*(paginas_usadas + pagina->numero) = getTimestamp(); // Actualizo el timestamp

	msleep(config.retardo_acc_mp);
}

t_segmento* buscar_segmento(char* nombre_tabla){
	bool buscador_tabla(t_segmento *segmento){
		return strcmp(segmento->nombre_tabla, nombre_tabla) == 0;
	}
	t_segmento* segmento = list_find(tabla_segmentos, (_Bool (*)(void*))buscador_tabla);

	return segmento;
}

t_pagina* buscar_pagina(t_segmento* segmento, uint16_t clave){
	t_list *tabla_paginas = segmento->paginas;
	bool buscador_pagina(t_pagina *pagina){
		uint16_t *key = (pagina->marco + DESPL_KEY);
		return *key == clave;
	}
	t_pagina *pagina = list_find(tabla_paginas, (_Bool (*)(void*))buscador_pagina);

	return pagina;
}

t_registro* buscar_registro(char* nombre_tabla, uint16_t key){
	// Busco si el segmento correspondiente existe en la tabla de segmentos
	t_segmento* segmento = buscar_segmento(nombre_tabla);
	if(!segmento){
		// El segmento para esa tabla todavia no existe en la tabla de segmentos
		log_debug(logger, "Todavia no existe un segmento para esa tabla");
		return NULL;
	}
	// El segmento para esa tabla ya existe en la tabla de segmentos

	// Busco si el registro ya existe en la tabla de paginas
	t_pagina *pagina = buscar_pagina(segmento, key);
	if(!pagina){
		// Todavia no existe una pagina para esa clave
		log_debug(logger, "Todavia no existe una pagina para esa clave");
		return NULL;
	}
	// Existe una pagina para esa clave
	t_registro* registro = malloc(sizeof(t_registro));
	*registro = leer_registro(pagina);
	return registro;
}

t_pagina* agregar_registro(uint16_t clave, char* valor, t_segmento* segmento){
	// Truncamos el valor al tamanio maximo posible para evitar problemas
	if(strlen(valor) > tamanio_value)
		valor[tamanio_value] = '\0';

	// Pedimos una pagina
	t_pagina* pagina = getPagina();
	if(!pagina){
		// La memoria esta full
		return NULL;
	}
	t_marco marco = pagina->marco;

	// Le cargamos los valores
	uint64_t *timestamp = marco + DESPL_TIMESTAMP;
	uint16_t *key = marco + DESPL_KEY;
	char *value = marco + DESPL_VALOR;

	*timestamp = getTimestamp();
	*key = clave;
	strcpy(value, valor);

	// La agregamos en la tabla de paginas
	t_list *tabla_paginas = segmento->paginas;
	list_add(tabla_paginas, pagina);

	*(paginas_usadas + pagina->numero) = getTimestamp(); // Actualizo el timestamp

	return pagina;
}

t_segmento* agregar_segmento(char* nombreTabla){
	// Creamos una tabla de paginas
	t_list* tabla_paginas = list_create();

	// Creamos el segmento
	t_segmento *segmento = malloc(sizeof(t_segmento));
	segmento->nombre_tabla = strdup(nombreTabla);
	segmento->paginas = tabla_paginas;

	// Lo agregamos a la tabla de segmentos
	list_add(tabla_segmentos, segmento);
	return segmento;
}

void vaciar_memoria(){
	log_trace(logger, "Realizando Journaling.");
	pthread_mutex_lock(&mutex_memoria_principal); // Evitamos que alguien tome nuevas paginas mientras tanto

	void liberar_segmento(t_segmento* segmento){
		free(segmento->nombre_tabla);
		free(segmento);
	}
	void iterador_paginas(t_pagina* pagina){
		if(pagina->modificado){
			// TODO: Persisto los cambios en FS
		}

		*(paginas_usadas + pagina->numero) = 0; // Indicamos que la pagina ahora esta libre
	}
	void iterador_segmentos(t_segmento* segmento){
		t_list* tabla_paginas = segmento->paginas;
		list_iterate(tabla_paginas, (void(*)(void*))iterador_paginas);
		list_destroy_and_destroy_elements(tabla_paginas, free);
	}
	list_iterate(tabla_segmentos, (void(*)(void*))iterador_segmentos);
	list_clean_and_destroy_elements(tabla_segmentos, (void(*)(void*))liberar_segmento);

	full = false;
	pthread_mutex_unlock(&mutex_memoria_principal);
	log_trace(logger, "Journaling terminado.");
}
