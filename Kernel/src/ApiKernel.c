#include "ApiKernel.h"
#include "Kernel.h"
#include "IPC.h"
#include "Memorias.h"
#include "Metricas.h"
#include "Misc.h"
#include "serializacion.h"

bool journaling (t_memoria* memoria){
	bool error_memoria = false;
	enum estados_journal respuesta = journalMemoria(memoria, &error_memoria);
	if(error_memoria){
		log_warning(logger, "ERROR: No nos pudimos conectar con la memoria %d. Asumiendo entonces que esta caida.", memoria->numero);
		return false;
	}
	if(respuesta != ESTADO_JOURNAL_OK){
		log_warning(logger, "No pudimos hacer journaling con la memoria %d.", memoria->numero);
		return false;
	}
	return true;
}


t_resultado selects(char* nombreTabla, u_int16_t key){
	unsigned long long inicio = getTimestamp(); // METRICAS

	t_resultado respuesta = { .falla = false };

	if(!existeTabla(nombreTabla)){
			respuesta.falla = true;
			respuesta.resultado = strdup("ERROR: Esa tabla no existe");
			return respuesta;
	}

	enum consistencias consistencia = obtener_consistencia(nombreTabla);

	struct_select paquete;
	paquete.key = key;
	paquete.nombreTabla = nombreTabla;

	struct_select_respuesta resultado;

	t_memoria * memoria;
	bool error_memoria;

	do {
		error_memoria = false;
		memoria = obtener_memoria_segun_consistencia(consistencia, key);
		if(!memoria){
			respuesta.falla = true;
			respuesta.resultado = strdup("ERROR: No tenemos una memoria asignada para ese tipo de consistencia");
			return respuesta;
		}

		log_trace(logger, "Enviando SELECT %s %d a la memoria %d", nombreTabla, key, memoria->numero);

		resultado = selectAMemoria(paquete, memoria, &error_memoria);

		if(error_memoria){
			log_warning(logger, "ERROR: No nos pudimos conectar con la memoria %d. Asumiendo entonces que esta caida, reintentamos con otra.", memoria->numero);
			eliminar_memoria(memoria);
		}
	} while (error_memoria);

	switch(resultado.estado) {
	case ESTADO_SELECT_OK:
		respuesta.resultado = resultado.valor;
		break;
	case ESTADO_SELECT_ERROR_TABLA:
		respuesta.resultado = strdup("ERROR: La tabla solicitada no existe.");
		break;
	case ESTADO_SELECT_ERROR_KEY:
		respuesta.resultado = strdup("ERROR: Esa tabla no contiene ningun registro con la clave solicitada.");
		break;
	case ESTADO_SELECT_MEMORIA_FULL:
		log_info(logger, "SELECT: La memoria esta FULL. Realizando JOURNAL");
		if(journaling(memoria)){
			log_info(logger, "SELECT: JOURNAL exitoso, repitiendo la request");
			respuesta = selects(nombreTabla, key);
		}
		else{
			respuesta.falla = true;
			respuesta.resultado = strdup("ERROR: Ocurrio un error durante el JOURNAL.");
		}
		break;
	default:
		respuesta.resultado = strdup("ERROR: Ocurrio un error desconocido.");
	}
	informar_select(consistencia, memoria->numero, inicio); // METRICAS
	free(memoria);
	return respuesta;
}

t_resultado insert(char* nombreTabla, u_int16_t key, char* valor){
	unsigned long long inicio = getTimestamp(); // METRICAS

	t_resultado respuesta;
	if(!existeTabla(nombreTabla)){
		respuesta.falla = true;
		respuesta.resultado = strdup("ERROR: Esa tabla no existe");
		return respuesta;
	}

	respuesta.falla = false;

	struct_insert paquete;
	paquete.nombreTabla = nombreTabla;
	paquete.key = key;
	paquete.valor = valor;

	enum consistencias consistencia = obtener_consistencia(nombreTabla);

	t_memoria * memoria;
	enum estados_insert resultado;
	bool error_memoria;
	do {
		error_memoria = false;
		memoria = obtener_memoria_segun_consistencia(consistencia, key);
		if(!memoria){
			respuesta.falla = true;
			respuesta.resultado = strdup ("ERROR: No tenemos una memoria asignada para ese tipo de consistencia");
			return respuesta;
		}

		log_trace(logger, "Enviando INSERT %s %d %s a la memoria %d", nombreTabla, key, valor, memoria->numero);

		resultado = insertAMemoria(paquete, memoria, &error_memoria);
		if(error_memoria){
			log_warning(logger, "ERROR: No nos pudimos conectar con la memoria %d. Asumiendo entonces que esta caida, reintentamos con otra.", memoria->numero);
			eliminar_memoria(memoria);
		}
	} while (error_memoria);


	switch (resultado) {
	case ESTADO_INSERT_OK:
		respuesta.resultado = strdup("Valor insertado");
		break;
	case ESTADO_INSERT_ERROR_TABLA:
		respuesta.resultado = strdup("ERROR: Esa tabla no existe.");
		break;
	case ESTADO_INSERT_MEMORIA_FULL:
		log_info(logger, "INSERT: La memoria esta FULL. Realizando JOURNAL");
		if(journaling(memoria)){
			log_info(logger, "INSERT: JOURNAL exitoso, repitiendo la request");
			respuesta = insert(nombreTabla, key, valor);
		}
		else{
			respuesta.falla = true;
			respuesta.resultado = strdup("ERROR: Ocurrio un error durante el JOURNAL.");
		}
		break;
	default:
		respuesta.resultado = strdup("ERROR: Ocurrio un error desconocido.");
	}
	informar_insert(consistencia, memoria->numero, inicio); // METRICAS
	free(memoria);
	return respuesta;
}
t_resultado create(char* nombreTabla, enum consistencias tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	t_resultado respuesta;
	respuesta.falla = false;

	if(existeTabla(nombreTabla)){
		respuesta.falla = true;
		respuesta.resultado = strdup("ERROR: Esa tabla ya existe");
		return respuesta;
	}

	struct_create paquete;
	paquete.nombreTabla = nombreTabla;
	paquete.consistencia = tipoConsistencia;
	paquete.particiones = cantidadParticiones;
	paquete.tiempoCompactacion = compactionTime;

	enum estados_create resultado;
	bool error_memoria;
	do {
		error_memoria = false;
		t_memoria* memoria = obtener_memoria_random_del_pool();

		if(!memoria){
			respuesta.falla = true;
			respuesta.resultado = strdup ("ERROR: No hay memorias conocidas.");
			return respuesta;
		}

		resultado = createAMemoria(paquete, memoria, &error_memoria);
		if(error_memoria){
			log_warning(logger, "ERROR: No nos pudimos conectar con la memoria %d. Asumiendo entonces que esta caida, reintentamos con otra.", memoria->numero);
			eliminar_memoria(memoria);
		}
		else
			free(memoria);
	} while (error_memoria);

	struct_describe_respuesta metadatos = { .consistencia = tipoConsistencia, .particiones = cantidadParticiones, .tiempo_compactacion = compactionTime };

	switch (resultado) {
	case ESTADO_CREATE_OK:
		 // Lo agregamos a la metadata conocida.
		pthread_mutex_lock(&mutex_metadata);
		addMetadata(nombreTabla, &metadatos);
		pthread_mutex_unlock(&mutex_metadata);
		respuesta.resultado = strdup("Tabla creada");
		break;
	case ESTADO_CREATE_ERROR_TABLAEXISTENTE:
		respuesta.resultado = strdup("ERROR: Esa tabla ya existe.");
		break;
	default:
		respuesta.resultado = strdup("ERROR: Ocurrio un error desconocido.");
	}
	return respuesta;
}
t_resultado describe(char* nombreTabla){
	t_resultado respuesta;
	respuesta.falla = false;

	struct_describe paquete;
	paquete.nombreTabla = nombreTabla;

	struct_describe_respuesta resultado;
	bool error_memoria;
	do {
		error_memoria = false;
		t_memoria* memoria = obtener_memoria_random_del_pool();

		if(!memoria){
			respuesta.falla = true;
			respuesta.resultado = strdup ("ERROR: No hay memorias conocidas.");
			return respuesta;
		}

		resultado = describeAMemoria(paquete, memoria, &error_memoria);

		if(error_memoria){
			log_warning(logger, "ERROR: No nos pudimos conectar con la memoria %d. Asumiendo entonces que esta caida, reintentamos con otra.", memoria->numero);
			eliminar_memoria(memoria);
		}
		else
			free(memoria);
	} while (error_memoria);

	pthread_mutex_lock(&mutex_metadata);
	addMetadata(nombreTabla, &resultado); // Lo agregamos a la metadata conocida.
	pthread_mutex_unlock(&mutex_metadata);

	switch(resultado.estado){
	case ESTADO_DESCRIBE_OK:
		respuesta.resultado = string_from_format("Consistencia: %s, Particiones: %d, Tiempo de Compactacion: %ld", consistenciaAString(resultado.consistencia), resultado.particiones, resultado.tiempo_compactacion);
		break;
	case ESTADO_DESCRIBE_ERROR_TABLA:
		respuesta.resultado = strdup("ERROR: Esa tabla no existe.");
		break;
	default:
		respuesta.resultado = strdup("ERROR: Ocurrio un error desconocido.");
	}

	return respuesta;
}

t_resultado describe_global(){
	t_resultado respuesta;
	respuesta.falla = false;

	pthread_mutex_lock(&mutex_metadata);
	refreshMetadata();

	respuesta.resultado = string_new();
	void iterador_describes(char* nombre_tabla, t_metadata* metadata_tabla){
		string_append_with_format(&respuesta.resultado, "Tabla: %s Consistencia: %s, Particiones: %d, Tiempo de Compactacion: %ld\n", nombre_tabla, consistenciaAString(metadata_tabla->consistencia), metadata_tabla->particiones, metadata_tabla->tiempo_compactacion);
	}
	dictionary_iterator(metadata, (void(*)(char*,void*))iterador_describes);

	pthread_mutex_unlock(&mutex_metadata);

	return respuesta;
}

t_resultado drop(char* nombreTabla){
	t_resultado respuesta = { .falla = false };
	struct_drop paquete;
	paquete.nombreTabla = nombreTabla;

	if(!existeTabla(nombreTabla)){
		respuesta.falla = true;
		respuesta.resultado = strdup("ERROR: Esa tabla no existe");
		return respuesta;
	}
	enum consistencias consistencia = obtener_consistencia(nombreTabla);

	enum estados_drop respuestaDrop;
	bool error_memoria;
	do {
		error_memoria = false;
		t_memoria * memoria = obtener_memoria_segun_consistencia(consistencia, 0);

		if(!memoria){
			respuesta.falla = true;
			respuesta.resultado = strdup ("ERROR: No tenemos una memoria asignada para ese tipo de consistencia");
			return respuesta;
		}

		respuestaDrop = dropTabla(paquete, memoria, &error_memoria);

		if(error_memoria){
			log_warning(logger, "ERROR: No nos pudimos conectar con la memoria %d. Asumiendo entonces que esta caida, reintentamos con otra.", memoria->numero);
			eliminar_memoria(memoria);
		}
		else
			free(memoria);
	} while (error_memoria);

	if (respuestaDrop != ESTADO_DROP_OK){
		log_warning(logger, "No pudimos hacer drop con la tabla %s.", nombreTabla);
		respuesta.falla = true;
	}

	respuesta.resultado = string_from_format("DROP realizado correctamente.");
	return respuesta;
}

t_resultado journal(){
	t_resultado respuesta;
	respuesta.falla = false;
	for (int consistencia = 0; consistencia < 3; ++consistencia) {
		list_iterate(listasMemorias[consistencia], (void (*)(void *))journaling);
	}
	respuesta.resultado = string_from_format("Journal realizado correctamente.");
	return respuesta;

}

t_resultado run(char* runPath){
	t_resultado respuesta;
	FILE * archivo = fopen (runPath, "r");
	if(!archivo){
		respuesta.falla = true;
		respuesta.resultado = strdup("ERROR: El archivo solicitado no existe.");
		return respuesta;
	}
	respuesta.falla = false;

	t_queue* requests = queue_create();
	char* request = NULL;
	size_t n = 0; // getline es jodon y pide que si no queres limitar el tamanio de lo que lee, igual le tenes que pasar un puntero a algo que valga 0...
	while(getline(&request, &n, archivo) > 0){
		string_trim(&request);
		if(!string_is_empty(request))
			queue_push(requests, request);

		request = NULL;
	}
	free(request);

	t_script *script = malloc(sizeof(t_script));
	script->requests = requests;
	script->nombre = strdup(runPath);
	aniadirScript(script);

	fclose (archivo);
	respuesta.resultado = strdup("Añadido script a ejecutar");
	return respuesta;
}

t_resultado metrics(){
	t_resultado respuesta;
	respuesta.falla = false;

	t_metricas * metricas = get_metricas();

	respuesta.resultado = strdup("METRICAS:");

	for (int consistencia = 0; consistencia < 3; ++consistencia) {
		string_append_with_format(&respuesta.resultado, "\n%s:\n\tRead latency: %lldms\n\tWrite latency: %lldms\n\tReads: %ld\n\tWrites: %ld",
				consistenciaAString(consistencia), metricas[consistencia].read_latency, metricas[consistencia].write_latency, metricas[consistencia].reads, metricas[consistencia].writes);

		void iterador(char* memoria, float* memory_load){
			string_append_with_format(&respuesta.resultado, "\n\tMemory %s Load: %.2f%%", memoria, (*memory_load) * 100);
		}
		dictionary_iterator(metricas[consistencia].memory_load, (void(*)(char*,void*)) iterador);
	}

	return respuesta;
}
t_resultado add(uint16_t numeroMemoria, enum consistencias criterio){
	t_resultado respuesta;
	t_memoria* memoria = getMemoria(numeroMemoria);

	if(memoria){
		respuesta.falla = false;
		bool buscador(t_memoria* otraMemoria){
			return memoria->numero == otraMemoria->numero;
		}
		pthread_mutex_lock(&mutex_pool_memorias);
		if(list_find(listasMemorias[criterio], (_Bool(*)(void*)) buscador)){
			respuesta.resultado = string_from_format("La memoria %d ya estaba asociada al criterio %s", memoria->numero, consistenciaAString(criterio));
			free(memoria);
		}
		else{
			if(criterio == SC && !list_is_empty(listasMemorias[criterio])){ // En el criterio SC solo tenemos una memoria asignada al mismo tiempo.
				list_clean_and_destroy_elements(listasMemorias[criterio], free);
			}
			list_add(listasMemorias[criterio], memoria);
			respuesta.resultado = string_from_format("Memoria %d asociada al criterio %s", memoria->numero, consistenciaAString(criterio));
		}
		pthread_mutex_unlock(&mutex_pool_memorias);
	}
	else{
		respuesta.falla = true;
		respuesta.resultado = strdup("Memoria invalida");
	}
	return respuesta;
}



