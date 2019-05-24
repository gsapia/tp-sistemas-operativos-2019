#include "IPC.h"
#include "Kernel.h"

//SOCKET CLIENTE (Dado que Kernel solo es Cliente con Memoria)

void cliente(){

         log_trace(logger, "Iniciando cliente kernel");

	     int cliente;

		 struct sockaddr_in direccionServidor;
		 direccionServidor.sin_family = AF_INET;
		 direccionServidor.sin_addr.s_addr = "192.168.1.2"; //Direccion IP
		 direccionServidor.sin_port = htons(8001); //Puerto al que me conecto (Memoria)


		 log_trace(logger, "Conectando con Memoria en %s:%d","192.168.1.2","8001");


	     cliente = socket(AF_INET, SOCK_STREAM, 0);//Pedimos un socket enviandole parametros que especifica que utilizamos protocolo TCP/ IP
	     while(connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor))){
	    	   log_trace(logger, "No se pudo conectar con el servidor (Memoria). Reintentando en 5 segundos.");
	    		sleep(5);
	    	}


	    //************ Conexion Kernel - Memoria (HandShake)***************


	    //Envio un primer mensaje

	    const uint8_t k = 1;
	    send(socket,&k, sizeof(k),0 );

	    //Recibo confirmacion de que el otro extremo es Memoria

	    uint8_t *otro= malloc (sizeof(uint8_t));

	    if(!(recv(socket, otro, sizeof(uint8_t ),0) && *otro == 2)) // Confirmo que el otro extremo es Memoria
	    {

	    //Si el otro extremo no es Memoria, cierro la conexion tirando un log y termino el programa.

	    	log_error(logger,"Error al conectar con Memoria");
	    	exit(EXIT_FAILURE);

	    }

        //El otro extremo es Memoria realmente asi que ahora enviamos / recibimos los datos necesarios.

        //En este caso recibimos el tamaño del value

	    uint16_t *tamanio_value = malloc(sizeof(uint16_t));
	    recv(socket,tamanio_value, sizeof (tamanio_value), 0);

	    log_trace(logger,"Contectado a Memoria !");

        //Fin de HandShake - Ahora podemos realizar solicitudes a Memoria.


	    //**Fin conexion Kernel-Memoria**//

} //End Cliente
