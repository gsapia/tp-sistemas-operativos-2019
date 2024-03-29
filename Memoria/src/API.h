#ifndef API_H_
#define API_H_

#include "Memoria.h"
#include "Misc.h"
#include "serializacion.h"

struct_select_respuesta selects(char* nombreTabla, u_int16_t key);
// ^ La funcion select() ya existe, hay que buscar otro nombre mas creativo.... ^
enum estados_insert insert(char* nombreTabla, u_int16_t key, char* valor);
enum estados_create create(char* nombreTabla, enum consistencias tipoConsistencia, uint16_t cantidadParticiones, uint32_t compactionTime);
struct_describe_respuesta describe(char* nombreTabla);
struct_describe_global_respuesta describe_global();
enum estados_drop drop(char* nombreTabla);
enum estados_journal journal();
char* estado(void);

#endif /* API_H_ */
