#include "Memorias.h"
#include "IPC.h"


t_memoria* getMemoria(int numero){
	bool buscador_memoria(t_memoria* memoria){
		return memoria->numero == numero;
	}
	t_memoria* memoria = list_find(pool_memorias, (_Bool(*)(void*)) buscador_memoria);
	if(!memoria){
		return NULL;
	}

	t_memoria* resultado = malloc(sizeof(t_memoria));
	memcpy(resultado, memoria, sizeof(t_memoria));

	return resultado; // Devolvemos una copia, para no joder la lista actual
}

bool existeTabla(char* nombre_tabla){
	// TODO
	return true;
}
t_memoria* obtener_memoria_random(){
	// TODO
	return NULL;
}
t_memoria* obtener_memoria_SC(){
	return list_get(listasMemorias[SC], 0);
}
t_memoria* obtener_memoria_SHC(uint16_t key){
	// TODO:
	return NULL;
}
t_memoria* obtener_memoria_EC(){
	// TODO:
	return NULL;
}
t_memoria* obtener_memoria_segun_tabla(char* nombre_tabla){
	// TODO
	return NULL;
}

void gossip(){
	while(1){
		int socket = conectar(config.ip_memoria, config.puerto_memoria);
		if(!socket){
			log_trace(logger, "No nos pudimos conectar con la memoria principal para pedir su tabla de gossip.");
		}
		else{
			// Pedimos la tabla de gossiping
			const uint8_t cod_op = GOSSIP;
			send(socket, &cod_op, sizeof(cod_op), 0);

			pool_memorias = recibir_tabla_gossiping(socket);

			// Ahora hay que eliminar las memorias que ya no existen de los criterios
			bool filtrador(t_memoria* memoria){
				bool son_iguales(t_memoria* otra_memoria){
					return otra_memoria->numero == memoria->numero;
				}
				return list_find(pool_memorias, (_Bool(*)(void*)) son_iguales);
			}
			for(int i = 0; i < 3; ++i){
				t_list* nueva = list_filter(listasMemorias[i], (_Bool(*)(void*)) filtrador);
				list_destroy_and_destroy_elements(listasMemorias[i], free);
				listasMemorias[i] = nueva;
			}

			char* memorias_conocidas = string_new();
			void iterador(t_memoria* memoria){
				string_append_with_format(&memorias_conocidas, "%d ", memoria->numero);
			}
			list_iterate(pool_memorias, (void (*)(void*)) iterador);
			string_trim(&memorias_conocidas);

			log_trace(logger, "Tabla de memorias actualizada. Memorias conocidas: %s", memorias_conocidas);
		}

		usleep(config.retardo_gossiping * 1000);
	}

}
