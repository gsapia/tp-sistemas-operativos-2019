#include "Memorias.h"
#include "IPC.h"


t_memoria* getMemoria(int numero){
	pthread_mutex_lock(&mutex_pool_memorias);
	if(!pool_memorias){
		// El pool todavia no fue inicializado
		pthread_mutex_unlock(&mutex_pool_memorias);
		return NULL;
	}
	bool buscador_memoria(t_memoria* memoria){
		return memoria->numero == numero;
	}
	t_memoria* memoria = list_find(pool_memorias, (_Bool(*)(void*)) buscador_memoria); //Agarra el pool de memorias, levanta la primera y la busca con la funcion
	if(!memoria){ 																	   //retorna si o no la encuentra, en caso de false vuelve a buscar hasta que sea true
		pthread_mutex_unlock(&mutex_pool_memorias);								  	   //si la encuentra retorna un puntero ,si no retorna NULL.
		return NULL;
	}

	t_memoria* resultado = malloc(sizeof(t_memoria));
	memcpy(resultado, memoria, sizeof(t_memoria));

	pthread_mutex_unlock(&mutex_pool_memorias);
	return resultado; // Devolvemos una copia, para no joder la lista actual
}

enum consistencias obtener_consistencia(char* nombre_tabla){
	pthread_mutex_lock(&mutex_metadata);
	t_metadata* metadata_tabla = dictionary_get(metadata, nombre_tabla);
	pthread_mutex_unlock(&mutex_metadata);
	if(metadata_tabla){
		return metadata_tabla->consistencia;
	}
	return -1;
}

bool existeTabla(char* nombre_tabla){
	pthread_mutex_lock(&mutex_metadata);
	bool resultado = dictionary_has_key(metadata, nombre_tabla);
	pthread_mutex_unlock(&mutex_metadata);
	return resultado;
}
t_memoria* obtener_memoria_random(t_list* lista_memorias){
	pthread_mutex_lock(&mutex_pool_memorias);
	if(!lista_memorias || list_is_empty(lista_memorias)){
		pthread_mutex_unlock(&mutex_pool_memorias);
		return NULL;
	}
	int cantidad = list_size(lista_memorias);
	int indice = rand() % cantidad;
	t_memoria* memoria = list_get(lista_memorias, indice);

	t_memoria* resultado = malloc(sizeof(t_memoria));
	memcpy(resultado, memoria, sizeof(t_memoria));

	pthread_mutex_unlock(&mutex_pool_memorias);
	return resultado; // Devolvemos una copia, para no joder la lista actual
}
t_memoria* obtener_memoria_random_del_pool(){
	return obtener_memoria_random(pool_memorias);
}
t_memoria* obtener_memoria_SC(){
	pthread_mutex_lock(&mutex_pool_memorias);
	t_memoria * memoria = list_get(listasMemorias[SC], 0);
	t_memoria* resultado = NULL;
	if(memoria){
		resultado = malloc(sizeof(t_memoria));
		memcpy(resultado, memoria, sizeof(t_memoria));
	}
	pthread_mutex_unlock(&mutex_pool_memorias);
	return resultado; // Devolvemos una copia, para no joder la lista actual
}
t_memoria* obtener_memoria_SHC(uint16_t key){
	t_memoria * resultado = NULL;
	pthread_mutex_lock(&mutex_pool_memorias);
	int cantidad = list_size(listasMemorias[SHC]);
	if(cantidad > 0){
		resultado = malloc(sizeof(t_memoria));
		memcpy(resultado, list_get(listasMemorias[SHC], key % cantidad), sizeof(t_memoria));  // Devolvemos una copia, para no joder la lista actual
	}
	pthread_mutex_unlock(&mutex_pool_memorias);
	return resultado;
}
t_memoria* obtener_memoria_EC(){
	return obtener_memoria_random(listasMemorias[EC]);
}
t_memoria* obtener_memoria_segun_consistencia(enum consistencias consistencia, uint16_t key){
	switch(consistencia){
	case SC:
		return obtener_memoria_SC();
	case SHC:
		return obtener_memoria_SHC(key);
	case EC:
		return obtener_memoria_EC();
	}
	log_debug(logger, "Consistencia invalida");
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

			pthread_mutex_lock(&mutex_pool_memorias);
			pool_memorias = recibir_tabla_gossiping(socket);
			close(socket);

			// Ahora hay que eliminar las memorias que ya no existen de los criterios
			for(int i = 0; i < 3; ++i){
				bool filtrador(t_memoria* memoria){
					bool son_iguales(t_memoria* otra_memoria){
						return otra_memoria->numero == memoria->numero;
					}
					return list_find(listasMemorias[i], (_Bool(*)(void*)) son_iguales);
				}
				t_list* nueva = list_filter(pool_memorias, (_Bool(*)(void*)) filtrador);
				list_destroy_and_destroy_elements(listasMemorias[i], free);
				listasMemorias[i] = nueva;
			}

			char* memorias_conocidas = string_new();
			void iterador(t_memoria* memoria){
				string_append_with_format(&memorias_conocidas, "%d (%s:%d) ", memoria->numero, memoria->IP, memoria->puerto);
			}
			list_iterate(pool_memorias, (void (*)(void*)) iterador);
			string_trim(&memorias_conocidas);
			pthread_mutex_unlock(&mutex_pool_memorias);

			log_trace(logger, "Tabla de memorias actualizada. Memorias conocidas: %s", memorias_conocidas);
		}
		sem_post(&primer_gossip_hecho);
		usleep(config.retardo_gossiping * 1000);
	}

}
