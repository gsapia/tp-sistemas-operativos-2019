#include "ApiKernel.h"
#include "Kernel.h"
#include "IPC.h"
#include "serializacion.h"

char* selects(char* nombreTabla, u_int16_t key){
	struct_select paquete;
	paquete.key = key;
	paquete.nombreTabla = nombreTabla;

	struct_select_respuesta resultado = selectAMemoria(paquete);

	switch(resultado.estado) {
	case ESTADO_SELECT_OK:
		return strdup(resultado.valor);
	case ESTADO_SELECT_ERROR_TABLA:
		return strdup("ERROR: La tabla solicitada no existe.");
	case ESTADO_SELECT_ERROR_KEY:
		return strdup("ERROR: Esa tabla no contiene ningun registro con la clave solicitada.");
	default:
		return strdup("ERROR: Ocurrio un error desconocido.");
	}
}
char* insert(char* nombreTabla, u_int16_t key, char* valor){
	struct_insert paquete;
	paquete.nombreTabla = nombreTabla;
	paquete.key = key;
	paquete.valor = valor;

	enum estados_insert resultado = insertAMemoria(paquete);

	switch (resultado) {
	case ESTADO_INSERT_OK:
		return strdup("Valor insertado");
	case ESTADO_INSERT_ERROR_TABLA:
		return strdup("ERROR: Esa tabla no existe.");
	default:
		return strdup("ERROR: Ocurrio un error desconocido.");
	}
}
char* create(char* nombreTabla, enum consistencias tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	struct_create paquete;
	paquete.nombreTabla = nombreTabla;
	paquete.consistencia = tipoConsistencia;
	paquete.particiones = cantidadParticiones;
	paquete.tiempoCompactacion = compactionTime;

	enum estados_create resultado = createAMemoria(paquete);

	switch (resultado) {
	case ESTADO_CREATE_OK:
		return strdup("Tabla creada");
	case ESTADO_CREATE_ERROR_TABLAEXISTENTE:
		return strdup("ERROR: Esa tabla ya existe.");
	default:
		return strdup("ERROR: Ocurrio un error desconocido.");
	}
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

char* run(char* runPath){
	FILE * archivo = fopen (runPath, "r");
	if(!archivo){
		return strdup("ERROR: El archivo solicitado no existe.");
	}

	t_queue* requests = queue_create();
	char* request = NULL;
	size_t n = 0; // getline es jodon y pide que si no queres limitar el tamanio de lo que lee, igual le tenes que pasar un puntero a algo que valga 0...
	while(getline(&request, &n, archivo) > 0){
		string_trim(&request);
		if(!string_is_empty(request))
			queue_push(requests, request);

		request = NULL;
	}

	t_script *script = malloc(sizeof(t_script));
	script->requests = requests;
	aniadirScript(script);

	fclose (archivo);
	return strdup("Añadido script a ejecutar");
}

char* metrics(){
	return string_from_format("Elegiste METRICS");
}

char* add(uint16_t numeroMemoria,enum consistencias criterio){
	return string_from_format("Elegiste ADD");
}




