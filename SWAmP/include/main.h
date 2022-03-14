/*
 * main.h
 *
 *  Created on: 18 sep. 2021
 *      Author: utnso
 */

#ifndef SWAMP_INCLUDE_MAIN_H_
#define SWAMP_INCLUDE_MAIN_H_

#include "protocolo_swap.h"
#include "declaraciones.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>

t_log* logger;
t_config* config;

//void* atender_memoria(void* cliente_fd);
void crear_archivos();
void inicializar_config();
int buscar_archivo(swap_petition cont_pag);
void escribir_archivo(int archivo_selec, swap_petition cont_pag);
void escribir_archivo_fijo(int archivo_selec, swap_petition cont_pag);
arch_bloq buscar_bloque(swap_petition pag_recib);
void* leer_archivo(arch_bloq arch_bloq);
void liberar_bloques(uint32_t mate_id);
void liberar_bloque(arch_bloq bloq_borrar);
void free_bloque(bloque_archivo* bloque);
void free_archivo(mate_file* archivo);
void free_memoria();

#endif /* KERNEL_INCLUDES_MAIN_H_ */
