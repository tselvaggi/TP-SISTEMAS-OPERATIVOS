/*
 * protocolos_lib.h
 *
 *  Created on: 28 nov. 2021
 *      Author: utnso
 */

#ifndef CARPINCHOS_INCLUDE_PROTOCOLOS_LIB_H_
#define CARPINCHOS_INCLUDE_PROTOCOLOS_LIB_H_

#include "matelib.h"
#include "../../shared/include/mensajeria.h"
#include <stdint.h>

void send_init(mate_instance* carpincho, int socket);
void recv_exec(mate_instance* carpincho, int socket);

void send_sem_init(mate_instance* carpincho, char* sem, unsigned int value, int socket);
void send_sem_operation(mate_instance* carpincho, char* sem, op_code code_op, int socket);
void send_io_usage(mate_instance* carpincho, char* io, void* msg, int socket);

void send_mate_memalloc_size(mate_instance* carpincho, int size, int socket);
void send_mate_mem_addr(uint32_t mate_id, uint32_t addr, op_code code_op, int socket);
void send_mate_memread(mate_instance* carpincho, uint32_t origin, int size, int socket);
void send_mate_memwrite(mate_instance* carpincho, void* origin, uint32_t dest, int size, int socket);

int recv_memory_confirm(int socket);
void recv_memreaded(void* buffer_leido, int socket);
uint32_t recv_mate_mem_alloced(int socket);


#endif /* CARPINCHOS_INCLUDE_PROTOCOLOS_LIB_H_ */
