#include "Gossiping.h"
#include <stdbool.h>
#include "IPC.h"
#include "Misc.h"

void agregar_memoria(t_memoria* memoria){
	bool buscador_memoria(t_memoria* otra_memoria){
		return !strcmp(memoria->IP, otra_memoria->IP) && memoria->puerto == otra_memoria->puerto;
	}
	// Solo la agregamos si no existe todavia
	if(!list_find(tabla_gossiping, (_Bool (*)(void*))buscador_memoria)){
		log_debug(logger, "Conocida memoria: %d ", memoria->numero);
		t_memoria* duplicado = malloc(sizeof(t_memoria));
		memcpy(duplicado, memoria, sizeof(t_memoria));
		list_add(tabla_gossiping, duplicado);
	}
}

void eliminar_memoria(t_memoria memoria){
	bool buscador_memoria(t_memoria* otra_memoria){
		return !strcmp(memoria.IP, otra_memoria->IP) && memoria.puerto == otra_memoria->puerto;
	}
	void eliminador(void* dato){
		t_memoria* memoria = dato;
		log_trace(logger, "GOSSIPING: La memoria %d se cayo.", memoria->numero);
		free(dato);
	}
	list_remove_and_destroy_by_condition(tabla_gossiping, (_Bool (*)(void*))buscador_memoria, eliminador);
}

void agregar_memorias(t_list* memorias){
	list_iterate(memorias, (void (*)(void*))agregar_memoria);
}

void hacer_gossiping(t_memoria memoria){
	log_trace(logger, "GOSSIPING: Intercambiando tablas con la memoria en %s:%d ...", memoria.IP, memoria.puerto);
	t_list* tabla = intercambiar_tabla_gossiping(memoria);

	// Si pudimos obtener su tabla de gossiping, agregamos las memorias que tenia. Sino, esa memoria no esta conectada y debemos eliminarla de nuestra tabla
	if(tabla){
		agregar_memorias(tabla);
		list_destroy_and_destroy_elements(tabla, free);
	}
	else{
		log_trace(logger, "GOSSIPING: No nos pudimos conectar con la memoria en %s:%d", memoria.IP, memoria.puerto);
		eliminar_memoria(memoria);
	}
}

void gossiping(){
	// Inicializo el sistema de gossiping
	tabla_gossiping = list_create();
	t_memoria * yo = malloc(sizeof(t_memoria));
	yo->numero = config.numero_memoria;
	yo->IP = "0.0.0.0"; // TODO: Arreglar esto
	yo->puerto = config.puerto_escucha;
	list_add(tabla_gossiping, yo);

	if(!config.ip_seeds[0]){ // No hay seeds configurados, por lo tanto es al pedo hacer gossiping
		log_trace(logger, "GOSSIPING: No hay seeds configuradas.");
		return;
	}

	while(1){
		log_trace(logger, "Iniciando ronda de Gossiping");

		for(int i = 0; config.ip_seeds[i]; i++){
			t_memoria memoria;
			memoria.IP = config.ip_seeds[i];
			memoria.puerto = config.puertos_seeds[i];

			hacer_gossiping(memoria);
		}

		char* memorias_conocidas = string_new();
		void iterador(t_memoria* memoria){
			string_append_with_format(&memorias_conocidas, "%d ", memoria->numero);
		}
		list_iterate(tabla_gossiping, (void (*)(void*)) iterador);
		string_trim(&memorias_conocidas);

		log_trace(logger, "Ronda de Gossiping finalizada. Memorias conocidas: %s", memorias_conocidas);
		msleep(config.tiempo_gossiping);
	}

}
