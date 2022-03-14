#include "../include/servidor.h"

int iniciar_servidor(char *ip, char *puerto)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto, &hints, &servinfo);

    // Creamos el socket de escucha del servidor
    socket_servidor = socket(servinfo->ai_family,
            				 servinfo->ai_socktype,
							 servinfo->ai_protocol);

    // Asociamos el socket a un puerto
    if(bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen)!=0){
    	//log_info(logger, "Error asociando el puerto");
    	exit(0);
    }

    // Escuchamos las conexiones entrantes
    listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    //log_info(logger, "---Listo para escuchar a cliente---");

    return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	// Aceptamos un nuevo cliente
	//log_info(logger, "Voy a aceptar a cliente");

	int socket_cliente = accept(socket_servidor, &dir_cliente, &tam_direccion);

	//log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}
