#include "Shared.h"

bool esArchivoValido(char* nombre_archivo){
	return strcmp(nombre_archivo,".") && strcmp(nombre_archivo,"..") && strcmp(nombre_archivo, "Metadata");
}
