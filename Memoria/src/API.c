#include "API.h"
#include "MemoriaPrincipal.h"
#include "IPC.h"

// TODO: Posible condicion de carrera cuando varios hilos quieran acceder a lo mismo!!
t_pagina* agregar_registro(uint16_t clave, char* valor, t_list *tabla_paginas){
	// Truncamos el valor al tamanio maximo posible para evitar problemas
	if(strlen(valor) > tamanio_value)
		valor[tamanio_value] = '\0';

	// Pedimos una pagina
	t_marco marco = getPagina();

	// Le cargamos los valores
	uint64_t *timestamp = marco + DESPL_TIMESTAMP;
	uint16_t *key = marco + DESPL_KEY;
	char *value = marco + DESPL_VALOR;

	*timestamp = getTimestamp();
	*key = clave;
	strcpy(value, valor);

	// La agregamos en la tabla de paginas
	t_pagina *pagina = malloc(sizeof(t_pagina));
	pagina->marco = marco;
	pagina->modificado = false;
	pagina->numero = list_size(tabla_paginas)+1; // Revisar bien

	list_add(tabla_paginas, pagina);

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

struct_select_respuesta selects(char* nombreTabla, u_int16_t key){
	log_debug(logger, "SELECT: Recibi Tabla:%s Key:%d", nombreTabla, key);

	// Busco si el segmento correspondiente existe en la tabla de segmentos
	bool buscador_tabla(t_segmento *segmento){
		return strcmp(segmento->nombre_tabla, nombreTabla) == 0;
	}
	t_segmento* segmento = list_find(tabla_segmentos, (_Bool (*)(void*))buscador_tabla);
	if(!segmento){
		// El segmento para esa tabla todavia no existe en la tabla de segmentos
		log_debug(logger, "Todavia no existe un segmento para esa tabla");

		// Agregamos el segmento para esa tabla
		segmento = agregar_segmento(nombreTabla);
	}
	// El segmento para esa tabla ya existe en la tabla de segmentos
	t_list *tabla_paginas = segmento->paginas;

	bool buscador_clave(t_pagina* pagina){
		uint16_t *clave = (pagina->marco + DESPL_KEY);
		return *clave == key;
	}

	t_pagina *pagina = list_find(tabla_paginas, (_Bool (*)(void*))buscador_clave);
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
		pagina = agregar_registro(key, respuesta.valor, tabla_paginas);
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
	bool buscador_tabla(t_segmento *segmento){
		return strcmp(segmento->nombre_tabla, nombreTabla) == 0;
	}
	t_segmento* segmento = list_find(tabla_segmentos, (_Bool (*)(void*))buscador_tabla);
	if(!segmento){
		// El segmento para esa tabla todavia no existe en la tabla de segmentos asi que lo agrego.
		log_debug(logger, "El segmento para esa tabla todavia no existe en la tabla de segmentos");

		// Agregamos el segmento para esa tabla
		segmento = agregar_segmento(nombreTabla);
	}
	// El segmento para esa tabla ya existe en la tabla de segmentos

	// Busco si el registro ya existe en la tabla de paginas
	t_list *tabla_paginas = segmento->paginas;
	bool buscador_pagina(t_pagina *pagina){
		uint16_t *key = (pagina->marco + DESPL_KEY);
		return *key == clave;
	}
	t_pagina *pagina = list_find(tabla_paginas, (_Bool (*)(void*))buscador_pagina);
	if(!pagina){
		// No tenemos el registro en la memoria principal, asi que tenemos que agregarlo
		log_debug(logger, "No tenemos este registro en la memoria principal");

		pagina = agregar_registro(clave, valor, tabla_paginas);
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
