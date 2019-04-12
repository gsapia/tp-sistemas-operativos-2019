#include "Kernel.h"

int main(void) {
	puts("Soy Kernel");

	int socketPedido;
		struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = INADDR_ANY;
		direccionServidor.sin_port = htons(6060); //Puerto que tiene que escuchar


	    socketPedido = socket (AF_INET ,SOCK_STREAM ,0); //Pedimos un socket enviandole parametros que especifica que utilizamos protocolo TCP/ IP

	    if (bind(socketPedido, (void*) &direccionServidor ,sizeof(direccionServidor)) != 0) //Colomos un bind para que no rompa el codigo.
	    {                                                                                   //Si falla me devuelve un codigo != de cero.
	    	perror("Fallo el bind!");
	    	return 1;

	    }//End if

	    printf("Estoy escuchando\n");
	    listen(socketPedido, SOMAXCONN); // Le pasamos como parametros el socket y un maximo de conexiones, esas conexiones iran a una cola
	                                    // las cuales iremos aceptando. (SOMAXCONN es el numero maximo de conexiones)


	return EXIT_SUCCESS;
}
