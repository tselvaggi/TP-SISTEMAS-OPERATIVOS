/*
 * memcalculate.h
 *
 *  Created on: 24 nov. 2021
 *      Author: utnso
 */

#ifndef MEMORIA_INCLUDE_MEMCALCULATE_H_
#define MEMORIA_INCLUDE_MEMCALCULATE_H_

#include "main.h"

int paginas_a_escribir(int pos_init, int pos_fin);

int cantidad_paginas_en_memoria(page_table* tabla);
int calcular_pagina(int pos);
int calcular_desplazamiento(int pos);
uint32_t calcular_dirFisica(pagina* page, uint32_t dir_logica, page_table* tabla);
bool direccion_valida(uint32_t direccion, page_table* tabla);

TLB* tiempo_menor_LRU_TLB(TLB* primero, TLB* segundo);
TLB* tiempo_menor_FIFO(TLB* primero, TLB* segundo);

pagina* LRU_MMU_FIJA(pagina* primero, pagina* segundo);
marcos_memoria* LRU_MMU_GLOBAL(marcos_memoria* primero, marcos_memoria* segundo);

bool esta_ocupada(marcos_memoria* entrada);

bool compare_time(struct timespec time1, struct timespec time2);
ulong convertir_timespec(struct timespec tiempo);

#endif /* MEMORIA_INCLUDE_MEMCALCULATE_H_ */
