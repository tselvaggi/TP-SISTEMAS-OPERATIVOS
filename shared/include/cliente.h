/*
 * servidor_cliente.h
 *
 *  Created on: 13 oct. 2021
 *      Author: utnso
 */

#ifndef SHARED_INCLUDE_CLIENTE_H_
#define SHARED_INCLUDE_CLIENTE_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include"loggers_config.h"


int crear_conexion(char* ip, char* puerto);
void liberar_conexion(int socket_cliente);


#endif /* SHARED_INCLUDE_CLIENTE_H_ */
