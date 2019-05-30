#ifndef API_H_
#define API_H_

#include "Memoria.h"
#include "Misc.h"
#include "serializacion.h"

struct_select_respuesta selects(char* nombreTabla, u_int16_t key);
// ^ La funcion select() ya existe, hay que buscar otro nombre mas creativo.... ^
char* insert(char* nombreTabla, u_int16_t key, char* valor);
char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
// ^^ tipoConsistencia podria ser un enum?? ^^
char* describe(char* nombreTabla);
char* drop(char* nombreTabla);
char* journal();


#endif /* API_H_ */
