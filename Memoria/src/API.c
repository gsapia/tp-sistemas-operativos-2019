#include "API.h"
#include "MemoriaPrincipal.h"

char* selects(char* nombreTabla, u_int16_t key){
	log_debug(logger, "SELECT: Recibi Tabla:%s Key:%d", nombreTabla, key);

	// Busco si el segmento correspondiente existe en la tabla de segmentos
	bool buscador_tabla(char* nombre_tabla){
		return strcmp(nombre_tabla, nombreTabla) != 0;
	}
	t_segmento* segmento = list_find(tabla_segmentos, buscador_tabla);
	if(segmento){
		// El segmento para esa tabla ya existe en la tabla de segmentos
		log_debug(logger, "El segmento para esa tabla ya existe en la tabla de segmentos");

		bool buscador_clave(t_pagina* pagina){
			uint16_t *clave = (memoria_principal + pagina->marco + DESPL_KEY);
			return *clave == key;
		}

		t_pagina *pagina = list_find(segmento->paginas, buscador_clave);

		if(pagina){
			// Existe una pagina para esa clave
			log_debug(logger, "Existe una pagina para esa clave");

			return string_duplicate((memoria_principal + pagina->marco + DESPL_VALOR));

		}
		else{
			// Todavia no existe una pagina para esa clave
		}
	}
	else{
		// El segmento para esa tabla todavia no existe en la tabla de segmentos
	}

	return string_duplicate("Elegiste SELECT");
}
char* insert(char* nombreTabla, u_int16_t clave, char* valor){
	// Truncamos el valor al tamanio maximo posible para evitar problemas
	valor[tamanio_value] = '\0';

	log_debug(logger, "INSERT: Recibi Tabla:%s Key:%d Valor:%s", nombreTabla, clave, valor);

	// Busco si el segmento correspondiente existe en la tabla de segmentos
	bool buscador_tabla(t_segmento *segmento){
		return strcmp(segmento->nombre_tabla, nombreTabla) != 0;
	}
	t_segmento* segmento = list_find(tabla_segmentos, buscador_tabla);
	if(segmento){
		// El segmento para esa tabla ya existe en la tabla de segmentos

	}
	else{
		// El segmento para esa tabla todavia no existe en la tabla de segmentos asi que lo agrego.
		log_debug(logger, "El segmento para esa tabla todavia no existe en la tabla de segmentos");

		// Creamos una tabla de paginas
		t_list* tabla_paginas = list_create();

		// Pedimos una pagina
		t_marco marco = getPagina();

		// Le cargamos los valores
		uint64_t *timestamp = memoria_principal + marco;
		uint16_t *key = memoria_principal + marco + sizeof(*timestamp);
		char *value = memoria_principal + marco + sizeof(*timestamp) + sizeof(*key);

		*timestamp = getTimestamp();
		*key = clave;
		strcpy(value, valor);

		// La agregamos en la tabla de paginas
		t_pagina *pagina;
		pagina->marco = marco;
		pagina->modificado = true;
		pagina->numero = 0; // Revisar bien

		list_add(tabla_paginas, pagina);

		// Creamos el segmento
		t_segmento* segmento = malloc(sizeof(t_segmento));
		strcpy(segmento->nombre_tabla, nombreTabla);
		segmento->paginas = tabla_paginas;

		// Lo agregamos a la tabla de segmentos
		list_add(tabla_segmentos, segmento);
		return string_from_format("Valor insertado!");
	}
	return string_from_format("Elegiste INSERT");
}
char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	log_debug(logger, "CREATE: Recibi Tabla:%s TipoDeConsistencia:%s CantidadDeParticines:%d TiempoDeCompactacion:%d", nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);
	return string_from_format("Elegiste CREATE");
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
