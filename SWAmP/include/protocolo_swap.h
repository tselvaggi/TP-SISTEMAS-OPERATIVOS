/*
 * protocolos_swap.h
 *
 *  Created on: 28 nov. 2021
 *      Author: utnso
 */

#ifndef SWAMP_INCLUDE_PROTOCOLO_SWAP_H_
#define SWAMP_INCLUDE_PROTOCOLO_SWAP_H_

#include<unistd.h>
#include "../../shared/include/mensajeria.h"

typedef struct swap_petition{

	uint32_t mate_id;
	uint32_t numero_pagina;
	void* buffer;

}swap_petition;

int recv_tipo_asignacion(int socket);

void recv_swap_out(swap_petition* petition, int socket);
void recv_swap_in_petition(swap_petition* petition, int socket);
void recv_swap_delete_pag(swap_petition* petition, int socket);
void recv_swap_delete(uint32_t* mate_id, int socket);

void send_swap_in(void* buffer, int size, int socket);



#endif /* SWAMP_INCLUDE_PROTOCOLO_SWAP_H_ */
