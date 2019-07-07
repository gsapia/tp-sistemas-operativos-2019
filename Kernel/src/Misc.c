#include <stdlib.h>
#include "Misc.h"

char* consistenciaAString(enum consistencias consistencia){
	switch(consistencia){
		case SC:
			return "SC";
		case SHC:
			return "SHC";
		case EC:
			return "EC";
	}
	return NULL;
}
