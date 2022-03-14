/*
 * main.h
 *
 *  Created on: 23 oct. 2021
 *      Author: utnso
 */

#ifndef KERNEL_INCLUDE_MAIN_H_
#define KERNEL_INCLUDE_MAIN_H_

#include "../../shared/include/cliente.h"
#include "../../shared/include/servidor.h"
#include "../../shared/include/mensajeria.h"
#include "../../shared/include/librerias.h"
#include "protocolo_kernel.h"
#include "declaraciones.h"
#include "listas_administrativas.h"

void aniadir_a_cola_corto_plazo(kernel_proc *process);
void aniadir_a_cola_largo_plazo(kernel_proc *process);
kernel_proc *init_proceso(int cliente_cast, int conexion_memoria);
void *prox_proc_exec();
bool prox_rafaga_menor(void * process,void * process2);
float prox_tiempo_exec(kernel_proc * process);
void* atender_cliente(void* cliente_fd);
void* administrador_largo();
void set_suspended(kernel_proc *process);
void set_blocked(kernel_proc *process);
void set_ready(kernel_proc *process);
void set_exec(kernel_proc *process);
void init_adm_corto();
void* administrador_corto();
void* administrador_mediano();
int time_dif(struct timespec inicio);
io_proc *get_io_from_name(char *io);
bool estaSuspended(void * process);
kernel_proc * quitar_de_lista(t_list * lista,void *process);
bool estaReady(void * process);
bool detectar_deadlock(kernel_proc *proceso);
void * admin_deadlock();
void destruirProceso(kernel_proc *process);
bool mayor_id(void * process,void * process2);

//int conexion_memoria;

t_log *logger;
t_config *config;

kernel_cfg kernel_config;


#endif /* KERNEL_INCLUDE_MAIN_H_ */
