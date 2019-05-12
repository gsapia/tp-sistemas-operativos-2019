#ifndef API_H_
#define API_H_

#include "Memoria.h"

char* selects(char* nombreTabla, u_int16_t key);
// ^ La funcion select() ya existe, hay que buscar otro nombre mas creativo.... ^
char* insert(char* nombreTabla, u_int16_t key, char* valor);
char* create(char* nombreTabla, char* tipoConsistencia, u_int cantidadParticiones, u_int compactionTime);
// ^ Cantidad de particiones puede ser mas grande que u_int??? Lo mismo para compactionTime ^
// ^ Ademas mas abajo asumo que son mayores a 0, quizas esta mal? ^
// ^^ tipoConsistencia podria ser un enum?? ^^
char* describe(char* nombreTabla);
char* drop(char* nombreTabla);
char* journal();

#endif /* API_H_ */
