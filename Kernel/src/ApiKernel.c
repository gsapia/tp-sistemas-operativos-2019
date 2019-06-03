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
	log_debug(logger, "INSERT: Recibi Tabla:%s Key:%d Valor:%s", nombreTabla, key, valor);
	return string_from_format("Elegiste INSERT");
}
char* create(char* nombreTabla, enum consistencias tipoConsistencia, u_int cantidadParticiones, u_int compactionTime){
	log_debug(logger, "CREATE: Recibi Tabla:%s TipoDeConsistencia:%d CantidadDeParticines:%d TiempoDeCompactacion:%d", nombreTabla, tipoConsistencia, cantidadParticiones, compactionTime);

	struct_create paquete;
	paquete.nombreTabla = nombreTabla;
	paquete.consistencia = tipoConsistencia;
	paquete.particiones = cantidadParticiones;
	paquete.tiempoCompactacion = compactionTime;

	uint16_t resultado = createAMemoria(paquete);

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
	return string_from_format("Elegiste RUN ");
}

char* metrics()
{
	return string_from_format("Elegiste METRICS");
}

char* add(uint16_t numeroMemoria,enum consistencias criterio){
	return string_from_format("Elegiste ADD");
}




