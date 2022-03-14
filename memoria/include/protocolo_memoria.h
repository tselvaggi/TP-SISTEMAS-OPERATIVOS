#ifndef MEMORIA_INCLUDE_PROTOCOLO_MEMORIA_H_
#define MEMORIA_INCLUDE_PROTOCOLO_MEMORIA_H_

#include<unistd.h>
#include "../../shared/include/mensajeria.h"


typedef struct mem_petition
{
	uint32_t mate_id;
	int size;
	void* origin;
	uint32_t dir_logica;

}mem_petition;

void recv_init_memoria(uint32_t* mate_id, int socket);
void send_exec_memoria(uint32_t mate_id, int socket);

void recv_suspended_memoria(mem_petition* petition, int socket);

void recv_mate_memalloc(mem_petition* petition, int socket);
void send_mate_mem_alloced(uint32_t addr, int socket);
void recv_mate_mem_addr(mem_petition* petition, int socket);
void recv_mate_memread(mem_petition* petition, int socket);
void recv_mate_memwrite(mem_petition* petition, int socket);

void send_memory_confirm(uint32_t dir_logica, op_code code_op, int socket);
void send_mate_memerror(int memerror, int socket);
void send_memreaded(void* buffer, int size, int socket);

void send_tipo_asignacion(int tipo_asignacion, op_code code_op, int socket);

void send_swap_out(void* buffer, int size, uint32_t mate_id, uint32_t numero_pagina, int socket);
void send_swap_in_petition(uint32_t mate_id, uint32_t numero_pagina, int socket);
void recv_swap_in(void** bloque, int socket);
void send_swap_delete_pag(uint32_t mate_id, uint32_t numero_pagina, int socket);
void send_swap_delete(uint32_t mate_id, int socket);

#endif /* MEMORIA_INCLUDE_PROTOCOLO_MEMORIA_H_ */
