#include <stdlib.h>
#include <sys/time.h>
#include "Misc.h"

uint64_t getTimestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

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
