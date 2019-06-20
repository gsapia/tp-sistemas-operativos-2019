#ifndef APIKERNEL_H_
#define APIKERNEL_H_

#include "serializacion.h"
#include "Kernel.h"

char* selects(char* nombreTabla, u_int16_t key);
char* insert(char* nombreTabla, u_int16_t key, char* valor);
char* create(char* nombreTabla, enum consistencias tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
char* describe(char* nombreTabla);
char* drop(char* nombreTabla);
char* journal();
char* run(char* runPath);
char* metrics();
char* add(uint16_t numeroMemoria, enum consistencias criterio);

t_list* listasMemorias[3]; //Array de listas de memorias para el ADD

#endif /* APIKERNEL_H_ */
