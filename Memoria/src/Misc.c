#include "Misc.h"

uint64_t getTimestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

int msleep(int milisegundos){
	return usleep(milisegundos * 1000);
}
