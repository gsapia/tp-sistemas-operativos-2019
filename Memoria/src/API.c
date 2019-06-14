#include "API.h"
#include "MemoriaPrincipal.h"
#include "IPC.h"

// TODO: Cuando detectamos que no tenemos un segmento o una pagina en select, antes de crearlos/pedirlos, deberiamos constatar que FS realmente tiene el registro buscado
struct_select_respuesta selects(char* nombreTabla, u_int16_t key){
	log_debug(logger, "SELECT: Recibi Tabla:%s Key:%d", nombreTabla, key);

	// Busco si el segmento correspondiente existe en la tabla de segmentos
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
	}
	// Existe una pagina para esa clave

	struct_select_respuesta respuesta;
	respuesta.estado = ESTADO_SELECT_OK;
	respuesta.valor = pagina->marco + DESPL_VALOR;
	respuesta.timestamp = *((uint64_t*)(pagina->marco + DESPL_TIMESTAMP));

	return respuesta;
}
enum estados_insert insert(char* nombreTabla, u_int16_t clave, char* valor){
	// Truncamos el valor al tamanio maximo posible para evitar problemas
	if(strlen(valor) > tamanio_value)
		valor[tamanio_value] = '\0';

	log_debug(logger, "INSERT: Recibi Tabla:%s Key:%d Valor:%s", nombreTabla, clave, valor);

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
	}
	// Ya tenemos este registro en la memoria principal, asi que lo modificamos
	t_marco marco = pagina->marco;

	uint64_t *timestamp = marco + DESPL_TIMESTAMP;
	char *value = marco + DESPL_VALOR;

	*timestamp = getTimestamp();
	strcpy(value, valor);

	pagina->modificado = true;

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
	return string_from_format("Elegiste JOURNAL");
}
