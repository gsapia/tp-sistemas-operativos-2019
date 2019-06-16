#include "API.h"
#include "MemoriaPrincipal.h"
#include "IPC.h"

// TODO: Cuando detectamos que no tenemos un segmento o una pagina en select, antes de crearlos/pedirlos, deberiamos constatar que FS realmente tiene el registro buscado
struct_select_respuesta selects(char* nombreTabla, u_int16_t key){
	log_debug(logger, "SELECT: Recibi Tabla:%s Key:%d", nombreTabla, key);

	/*// Busco si el segmento correspondiente existe en la tabla de segmentos
	t_segmento* segmento = buscar_segmento(nombreTabla);
	if(!segmento){
		// El segmento para esa tabla todavia no existe en la tabla de segmentos
		log_debug(logger, "Todavia no existe un segmento para esa tabla");

		// Agregamos el segmento para esa tabla
		segmento = agregar_segmento(nombreTabla);
	}
	// El segmento para esa tabla ya existe en la tabla de segmentos

	// Busco si el registro ya existe en la tabla de paginas
	t_pagina *pagina = buscar_pagina(segmento, key);
	if(!pagina){
		// Todavia no existe una pagina para esa clave
		log_debug(logger, "Todavia no existe una pagina para esa clave");

		// Hago el SELECT a FS
		struct_select paquete;
		paquete.key = key;
		paquete.nombreTabla = nombreTabla;
		struct_select_respuesta respuesta = selectAFS(paquete);

		if(respuesta.estado != ESTADO_SELECT_OK){ // Si el resultado no fue el esperado, no guardo nada y devuelvo lo que recibi
			return respuesta;
		}

		// Agrego el registro
		pagina = agregar_registro(key, respuesta.valor, segmento);
		free(respuesta.valor);

		if(!pagina){
			struct_select_respuesta respuesta;
			respuesta.estado = ESTADO_SELECT_MEMORIA_FULL;
			return respuesta;
		}
	}
	// Existe una pagina para esa clave

	t_registro registro = leer_registro(pagina);
	struct_select_respuesta respuesta;
	respuesta.estado = ESTADO_SELECT_OK;
	respuesta.valor = registro.valor;
	respuesta.timestamp = registro.timestamp;*/

	struct_select_respuesta respuesta;
	pthread_mutex_lock(&mutex_memoria_principal); // Mutex para evitar que dos hilos distintos tomen la misma pagina sin querer, o que el valor sea modificado mientras se lee
	t_registro* registro = buscar_registro(nombreTabla, key);
	if(registro){
		respuesta.estado = ESTADO_SELECT_OK;
		respuesta.valor = registro->valor;
		respuesta.timestamp = registro->timestamp;
		free(registro);
	}
	else{
		// Hago el SELECT a FS
		struct_select paquete;
		paquete.key = key;
		paquete.nombreTabla = nombreTabla;
		respuesta = selectAFS(paquete);

		if(respuesta.estado == ESTADO_SELECT_OK){ // Si el resultado no fue el esperado, no guardo nada y devuelvo lo que recibI
			// Agrego el registro
			t_pagina* pagina = inicializar_nuevo_registro(nombreTabla, key, respuesta.valor);

			if(!pagina){
				respuesta.estado = ESTADO_SELECT_MEMORIA_FULL;
			}
		}
	}
	pthread_mutex_unlock(&mutex_memoria_principal);
	return respuesta;
}
enum estados_insert insert(char* nombreTabla, u_int16_t clave, char* valor){
	// Truncamos el valor al tamanio maximo posible para evitar problemas
	if(strlen(valor) > tamanio_value)
		valor[tamanio_value] = '\0';

	log_debug(logger, "INSERT: Recibi Tabla:%s Key:%d Valor:%s", nombreTabla, clave, valor);
	pthread_mutex_lock(&mutex_memoria_principal); // Mutex para evitar que dos hilos distintos tomen la misma pagina sin querer, o que la pagina se limpie mientras se modifica debido a un journal inoportuno
	// Busco si el segmento correspondiente existe en la tabla de segmentos
	t_segmento* segmento = buscar_segmento(nombreTabla);
	if(!segmento){
		// El segmento para esa tabla todavia no existe en la tabla de segmentos asi que lo agrego.
		log_debug(logger, "El segmento para esa tabla todavia no existe en la tabla de segmentos");

		// Agregamos el segmento para esa tabla
		segmento = agregar_segmento(nombreTabla);
	}
	// El segmento para esa tabla ya existe en la tabla de segmentos

	// Busco si el registro ya existe en la tabla de paginas
	t_pagina *pagina = buscar_pagina(segmento, clave);
	if(!pagina){
		// No tenemos el registro en la memoria principal, asi que tenemos que agregarlo
		log_debug(logger, "No tenemos este registro en la memoria principal");

		pagina = agregar_registro(clave, valor, segmento);

		if(!pagina){
			pthread_mutex_unlock(&mutex_memoria_principal);
			return ESTADO_INSERT_MEMORIA_FULL;
		}
	}
	// Ya tenemos este registro en la memoria principal, asi que lo modificamos
	modificar_registro(pagina, valor);
	pthread_mutex_unlock(&mutex_memoria_principal);
	return ESTADO_INSERT_OK;
}
enum estados_create create(char* nombreTabla, enum consistencias tipoConsistencia, uint16_t cantidadParticiones, uint32_t compactionTime){
	// Hago el CREATE a FS
	struct_create paquete;
	paquete.nombreTabla = nombreTabla;
	paquete.consistencia = tipoConsistencia;
	paquete.particiones = cantidadParticiones;
	paquete.tiempoCompactacion = compactionTime;

	enum estados_create resultado = createAFS(paquete);

	return resultado;
}
char* describe(char* nombreTabla){
	log_debug(logger, "DESCRIBE: Recibi Tabla:%s", nombreTabla);
	return string_from_format("Elegiste DESCRIBE");
}
char* drop(char* nombreTabla){
	log_debug(logger, "DROP: Recibi Tabla:%s", nombreTabla);
	return string_from_format("Elegiste DROP");
}
char* journal(){
	vaciar_memoria();
	return string_from_format("Elegiste JOURNAL");
}
