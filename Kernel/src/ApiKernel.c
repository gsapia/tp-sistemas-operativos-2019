#include "ApiKernel.h"
#include "Kernel.h"
#include "IPC.h"
#include "Memorias.h"
#include "serializacion.h"

t_resultado selects(char* nombreTabla, u_int16_t key){
	t_resultado respuesta;
	if(!existeTabla(nombreTabla)){
		respuesta.falla = true;
		respuesta.resultado = strdup("ERROR: Esa tabla no existe");
		return respuesta;
	}
	t_memoria * memoria = obtener_memoria_segun_tabla(nombreTabla, key);
	if(!memoria){
		respuesta.falla = true;
		respuesta.resultado = strdup("ERROR: No tenemos una memoria asignada para ese tipo de consistencia");
		return respuesta;
	}
	respuesta.falla = false;

	struct_select paquete;
	paquete.key = key;
	paquete.nombreTabla = nombreTabla;

	struct_select_respuesta resultado = selectAMemoria(paquete, memoria);
	free(memoria);

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
	default:
		respuesta.resultado = strdup("ERROR: Ocurrio un error desconocido.");
	}
	return respuesta;
}

t_resultado insert(char* nombreTabla, u_int16_t key, char* valor){
	t_resultado respuesta;
	if(!existeTabla(nombreTabla)){
		respuesta.falla = true;
		respuesta.resultado = strdup("ERROR: Esa tabla no existe");
		return respuesta;
	}
	t_memoria * memoria = obtener_memoria_segun_tabla(nombreTabla, key);
	if(!memoria){
		respuesta.falla = true;
		respuesta.resultado = strdup ("ERROR: No tenemos una memoria asignada para ese tipo de consistencia");
		return respuesta;
	}
	respuesta.falla = false;

	struct_insert paquete;
	paquete.nombreTabla = nombreTabla;
	paquete.key = key;
	paquete.valor = valor;

	enum estados_insert resultado = insertAMemoria(paquete, memoria);
	free(memoria);

	switch (resultado) {
	case ESTADO_INSERT_OK:
		respuesta.resultado = strdup("Valor insertado");
		break;
	case ESTADO_INSERT_ERROR_TABLA:
		respuesta.resultado = strdup("ERROR: Esa tabla no existe.");
		break;
	default:
		respuesta.resultado = strdup("ERROR: Ocurrio un error desconocido.");
	}
	return respuesta;
}
t_resultado create(char* nombreTabla, enum consistencias tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	t_resultado respuesta;
	respuesta.falla = false;

	struct_create paquete;
	paquete.nombreTabla = nombreTabla;
	paquete.consistencia = tipoConsistencia;
	paquete.particiones = cantidadParticiones;
	paquete.tiempoCompactacion = compactionTime;

	enum estados_create resultado = createAMemoria(paquete);

	switch (resultado) {
	case ESTADO_CREATE_OK:
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
	log_debug(logger, "DESCRIBE: Recibi Tabla:%s", nombreTabla);

	t_resultado respuesta;
	respuesta.falla = false;
	respuesta.resultado = string_from_format("Elegiste DESCRIBE");

	return respuesta;
}

t_resultado drop(char* nombreTabla){
	log_debug(logger, "DROP: Recibi Tabla:%s", nombreTabla);
	t_resultado respuesta;
	respuesta.falla = false;
	respuesta.resultado = string_from_format("Elegiste DROP");
	return respuesta;
}

t_resultado journal(){
	t_resultado respuesta;
	respuesta.falla = false;
	respuesta.resultado = string_from_format("Elegiste JOURNAL");
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
	respuesta.resultado = string_from_format("Elegiste METRICS");
	return respuesta;
}
t_resultado add(uint16_t numeroMemoria, enum consistencias criterio){
	log_debug(logger, "Criterio %d", criterio);
	t_resultado respuesta;
	t_memoria* memoria = getMemoria(numeroMemoria);

	if(memoria){
		list_add(listasMemorias[criterio], memoria);

		respuesta.falla = false;
		respuesta.resultado = strdup("Memoria Agregada!");
	}
	else{
		respuesta.falla = true;
		respuesta.resultado = strdup("Memoria invalida");
	}
	return respuesta;
}



