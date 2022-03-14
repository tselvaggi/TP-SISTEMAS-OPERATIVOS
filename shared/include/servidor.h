/*
 * servidor.h
 *
 *  Created on: 28 nov. 2021
 *      Author: utnso
 */

#ifndef SHARED_INCLUDE_SERVIDOR_H_
#define SHARED_INCLUDE_SERVIDOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include"loggers_config.h"

int iniciar_servidor(char*, char*);
int esperar_cliente(int);


#endif /* SHARED_INCLUDE_SERVIDOR_H_ */
