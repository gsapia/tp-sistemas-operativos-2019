#ifndef APIKERNEL_H_
#define APIKERNEL_H_

#include "Kernel.h"

char* selects(char* nombreTabla, u_int16_t key);
char* insert(char* nombreTabla, u_int16_t key, char* valor);
char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
char* describe(char* nombreTabla);
char* drop(char* nombreTabla);
char* journal();
char* run();
char* metrics();
char* add();

#endif /* APIKERNEL_H_ */
