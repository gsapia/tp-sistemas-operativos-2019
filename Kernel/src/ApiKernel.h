#ifndef APIKERNEL_H_
#define APIKERNEL_H_

#include "serializacion.h"
#include "Kernel.h"

t_resultado selects(char* nombreTabla, u_int16_t key);
t_resultado insert(char* nombreTabla, u_int16_t key, char* valor);
t_resultado create(char* nombreTabla, enum consistencias tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
t_resultado describe(char* nombreTabla);
t_resultado drop(char* nombreTabla);
t_resultado journal();
t_resultado run(char* runPath);
t_resultado metrics();
t_resultado add(uint16_t numeroMemoria, enum consistencias criterio);

t_list* listasMemorias[3]; //Array de listas de memorias para el ADD

#endif /* APIKERNEL_H_ */
