/*
 * protocolo_krenel.h
 *
 *  Created on: 28 nov. 2021
 *      Author: utnso
 */

#ifndef KERNEL_INCLUDE_PROTOCOLO_KERNEL_H_
#define KERNEL_INCLUDE_PROTOCOLO_KERNEL_H_

#include "declaraciones.h"
#include "../../shared/include/mensajeria.h"

void recv_init(uint32_t mate_id, int socket);
void send_exec(uint32_t mate_id, int socket);

//TODO: SEND EXEC A MEMORIA
void recv_exec_memoria(int conexion_memoria);

void recv_sem_init(kernel_sem* sem, int socket);
char* recv_sem_operation(int socket);
char* recv_io_usage(int socket);

void pasamanos_kernel_memoria(int socket_from, int socket_dest, op_code cod_op);

void send_suspended_memoria(uint32_t mate_id, int socket);
void send_init_memoria(uint32_t mate_id, int socket);

#endif /* KERNEL_INCLUDE_PROTOCOLO_KERNEL_H_ */
