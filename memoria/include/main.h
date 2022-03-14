/*
 * main.h
 *
 *  Created on: 24 oct. 2021
 *      Author: utnso
 */

#ifndef MEMORIA_INCLUDE_MAIN_H_
#define MEMORIA_INCLUDE_MAIN_H_


#include<commons/log.h>
#include<commons/config.h>
#include "../include/declaraciones.h"
#include <pthread.h>



#define TAM_METADATA 9

mem_cfg mem_config;

void* atender_cliente(void* cliente_fd);

#endif /* MEMORIA_INCLUDE_MAIN_H_ */
