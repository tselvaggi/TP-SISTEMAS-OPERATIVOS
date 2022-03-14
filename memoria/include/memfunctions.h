/*
 * memfunctions.h
 *
 *  Created on: 9 oct. 2021
 *      Author: utnso
 */

#ifndef INCLUDE_MEMFUNCTIONS_H_
#define INCLUDE_MEMFUNCTIONS_H_

#include "declaraciones.h"

int PAGE_SIZE, MEM_SIZE, TIPO_ASIGNACION, ALGORITMO_REEMPLAZO_MMU, ALGORITMO_REEMPLAZO_TLB, MARCOS_POR_CARPINCHO;

int TLB_HIT, TLB_MISS;

int INSERT;

void iniciar_memoria(void);

uint32_t memalloc(int size, page_table* tabla);

int memfree(uint32_t pos, page_table* tabla);

int memread(uint32_t direccion, int size, void* buffer, page_table* tabla);
void escribir_memoria(uint32_t pos, int size, void* data, page_table* tabla);

int memwrite(uint32_t direccion, int size, void* buffer, page_table* tabla);
void leer_memoria(uint32_t pos, int size, void* data, page_table* tabla);

bool direccion_valida(uint32_t direccion, page_table* tabla);

bool analizar_mate_error(int mate_error, int result);

#endif /* INCLUDE_MEMFUNCTIONS_H_ */
