#ifndef API_H_
#define API_H_

#include "Memoria.h"
#include "Misc.h"
#include "serializacion.h"

struct_select_respuesta selects(char* nombreTabla, u_int16_t key);
// ^ La funcion select() ya existe, hay que buscar otro nombre mas creativo.... ^
char* insert(char* nombreTabla, u_int16_t key, char* valor);
enum estados_create create(char* nombreTabla, enum consistencias tipoConsistencia, uint16_t cantidadParticiones, uint32_t compactionTime);
char* describe(char* nombreTabla);
char* drop(char* nombreTabla);
char* journal();


#endif /* API_H_ */
