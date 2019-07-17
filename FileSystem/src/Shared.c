#include "Shared.h"

bool esArchivoValido(char* nombre_archivo){
	return strcmp(nombre_archivo,".") && strcmp(nombre_archivo,"..") && strcmp(nombre_archivo, "Metadata");
}

uint64_t getTimestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

uint64_t stringToLong(char* strToInt){
	char* endptr, *str = strToInt;
	uint64_t numero = strtoul(str, &endptr, 10);
	return numero;
}
uint64_t stringToLongLong(char* strToInt){
	char* endptr, *str = strToInt;
	uint64_t numero = strtoull(str, &endptr, 10);
	return numero;
}

char* obtenerUltimaLinea(char* bloque){
	char* path = string_from_format("%sBloques/%s.bin",config.puntoMontaje, bloque);
	FILE* bloque_bin = fopen(path, "r");
	free(path); int renglon = 0;
	char* buffer_bloque_bin = NULL; size_t size_buffer_bloque_bin = 0;

	while(getline(&buffer_bloque_bin, &size_buffer_bloque_bin, bloque_bin) != -1){
		free(buffer_bloque_bin);
		buffer_bloque_bin = NULL;
		renglon++;
	}
	fseek(bloque_bin, 0, SEEK_SET);
	for(int j=0; j<renglon-1; j++){
		if(getline(&buffer_bloque_bin, &size_buffer_bloque_bin, bloque_bin) != -1){
			free(buffer_bloque_bin);
			buffer_bloque_bin = NULL;
		}
	}
	if(getline(&buffer_bloque_bin, &size_buffer_bloque_bin, bloque_bin) != -1){}

	fclose(bloque_bin);
	return buffer_bloque_bin;
}
